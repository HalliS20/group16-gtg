#include <cmath>
#include <cstdint>
#include <cstdio>
#include <print>
#include <unistd.h>
#include <vector>

#include "RTIMULib.h"
#include "circular_buffer.hpp"
#include "integral.hpp"
#include "linear_regression.hpp"

constexpr int buffer_length = 1000;

inline int run_rtimulib_example1()
{
	// int sampleCount = 0;
	// int sampleRate = 0;
	uint64_t rateTimer = 0;
	uint64_t displayTimer = 0;
	uint64_t now = 0;

	RTIMUSettings* settings = new RTIMUSettings("RTIMULib");

	RTIMU* imu = RTIMU::createIMU(settings);
	RTPressure* pressure = RTPressure::createPressure(settings);
	RTHumidity* humidity = RTHumidity::createHumidity(settings);

	if ((imu == NULL) || (imu->IMUType() == RTIMU_TYPE_NULL)) {
		std::println("No IMU found");
		exit(1);
	}

	imu->IMUInit();

	imu->setSlerpPower(0.02);
	imu->setGyroEnable(true);
	imu->setAccelEnable(true);
	imu->setCompassEnable(true);

	if (pressure != NULL) {
		pressure->pressureInit();
	}

	if (humidity != NULL) {
		humidity->humidityInit();
	}

	rateTimer = displayTimer = RTMath::currentUSecsSinceEpoch();

	const double G = 9.80665;

	const double accel_deadband = 0.05;

	const uint64_t rate_update_interval = 1000000; // us, update sample rate every 1 second

	cb::CircularBuffer<double> x_accel_buf(buffer_length);
	cb::CircularBuffer<double> y_accel_buf(buffer_length);
	cb::CircularBuffer<double> z_accel_buf(buffer_length);

	cb::CircularBuffer<double> x_vel_buf(buffer_length);
	cb::CircularBuffer<double> y_vel_buf(buffer_length);
	cb::CircularBuffer<double> z_vel_buf(buffer_length);

	// cb::CircularBuffer<double> x_pos_buf(buffer_length);
	// cb::CircularBuffer<double> y_pos_buf(buffer_length);
	// cb::CircularBuffer<double> z_pos_buf(buffer_length);

	cb::CircularBuffer<uint64_t> accel_ts(buffer_length);
	cb::CircularBuffer<uint64_t> vel_ts(buffer_length);
	// cb::CircularBuffer<uint64_t> pos_ts(buffer_length);

	double x_velocity = 0;
	double y_velocity = 0;
	double z_velocity = 0;
	double x_position = 0;
	double y_position = 0;
	double z_position = 0;

	// ---- Velocity timing -------------------------------------------------
	// This is the ONLY value you need to change to control the timing.
	// Everything else (interval, window size, integration time) is derived
	// from it and from the actual sensor timestamps.
	const uint64_t velocity_calculations_per_second = 10;

	const uint64_t velocity_calculation_interval = 1000000 / velocity_calculations_per_second;
	bool velocity_timer_started = false;
	uint64_t velocity_timer = 0;
	size_t samples_since_velocity = 0;

	// ---- Position timing -------------------------------------------------
	// This is the ONLY value you need to change to control the timing.
	const uint64_t position_calculations_per_second = 1;

	const uint64_t position_calculation_interval = 1000000 / position_calculations_per_second; // us

	bool position_timer_started = false;
	uint64_t position_timer = 0;
	size_t samples_since_position = 0;

	const uint64_t stationary_hold_time = 500000;
	bool stationary = false;
	uint64_t stationary_since = 0;

	lr::LineFit x_accel_line;
	lr::LineFit y_accel_line;
	lr::LineFit z_accel_line;
	lr::LineFit x_pos_line;
	lr::LineFit y_pos_line;
	lr::LineFit z_pos_line;

	while (true) {

		usleep(imu->IMUGetPollInterval() * 1000);

		while (imu->IMURead()) {

			RTIMU_DATA imuData = imu->getIMUData();

			if (imuData.accelValid) {
				RTVector3 linearAccel = imu->getAccelResiduals();

				double x_residual = linearAccel.x();
				double y_residual = linearAccel.y();
				double z_residual = linearAccel.z();

				// clearing out acceleration noise
				if (std::abs(x_residual) < accel_deadband) {
					x_residual = 0;
				}
				if (std::abs(y_residual) < accel_deadband) {
					y_residual = 0;
				}
				if (std::abs(z_residual) < accel_deadband) {
					z_residual = 0;
				}

				x_accel_buf.add(x_residual);
				y_accel_buf.add(y_residual);
				z_accel_buf.add(z_residual);
				accel_ts.add(imuData.timestamp);

				// clearing out velocity noise
				const double total_magnitude = std::sqrt((x_residual * x_residual)
					+ (y_residual * y_residual) + (z_residual * z_residual));

				// might want to set this accel deadband as seperate variable, this might be too aggressive
				if (total_magnitude >= accel_deadband) {
					stationary = false;
				} else if (!stationary) {
					stationary = true;
					stationary_since = imuData.timestamp;
				}
				if (stationary && (imuData.timestamp - stationary_since) >= stationary_hold_time) {
					x_velocity = y_velocity = z_velocity = 0;
				}

				samples_since_velocity++;

				if (!velocity_timer_started) {
					velocity_timer = imuData.timestamp;
					velocity_timer_started = true;
					samples_since_velocity = 0;
				}
			}

			// sampleCount++;

			now = RTMath::currentUSecsSinceEpoch();

			if (velocity_timer_started && imuData.accelValid
				&& (imuData.timestamp - velocity_timer) >= velocity_calculation_interval) {

				// Window = every sample since the last calculation, plus the last
				// sample of the previous window so no time span is skipped.
				const std::vector<uint64_t> window_ts = accel_ts.get(samples_since_velocity + 1);

				if (window_ts.size() >= 2) {
					// Window length in seconds (fitLine handles the offset/scaling itself)
					const double T
						= static_cast<double>(window_ts[window_ts.size() - 1] - window_ts[0]) * 1e-6;

					x_accel_line = lr::fitLine(x_accel_buf.get(window_ts.size()), window_ts);
					y_accel_line = lr::fitLine(y_accel_buf.get(window_ts.size()), window_ts);
					z_accel_line = lr::fitLine(z_accel_buf.get(window_ts.size()), window_ts);

					// Integral of acceleration (in g) over T seconds, converted to m/s
					x_velocity
						+= integral::linearIntegral(x_accel_line.slope, x_accel_line.intercept, T)
						* G;
					y_velocity
						+= integral::linearIntegral(y_accel_line.slope, y_accel_line.intercept, T)
						* G;
					z_velocity
						+= integral::linearIntegral(z_accel_line.slope, z_accel_line.intercept, T)
						* G;

					// Store velocity for the position calculation
					x_vel_buf.add(x_velocity);
					y_vel_buf.add(y_velocity);
					z_vel_buf.add(z_velocity);
					vel_ts.add(imuData.timestamp);

					samples_since_position++;
					// Start the position timer on the first velocity sample, using sensor time
					if (!position_timer_started) {
						position_timer = imuData.timestamp;
						position_timer_started = true;
						samples_since_position = 0;
					}
				}

				velocity_timer = imuData.timestamp;
				samples_since_velocity = 0;
			}

			if (position_timer_started && imuData.accelValid
				&& (imuData.timestamp - position_timer) >= position_calculation_interval) {

				// Window = every velocity sample since the last calculation, plus the last
				const std::vector<uint64_t> window_ts = vel_ts.get(samples_since_position + 1);

				if (window_ts.size() >= 2) {
					// Window length in seconds
					const double T
						= static_cast<double>(window_ts[window_ts.size() - 1] - window_ts[0]) * 1e-6;

					x_pos_line = lr::fitLine(x_vel_buf.get(window_ts.size()), window_ts);
					y_pos_line = lr::fitLine(y_vel_buf.get(window_ts.size()), window_ts);
					z_pos_line = lr::fitLine(z_vel_buf.get(window_ts.size()), window_ts);

					// Integral of velocity (m/s) over T seconds gives position in metres metres
					x_position += integral::linearIntegral(x_pos_line.slope, x_pos_line.intercept, T);
					y_position += integral::linearIntegral(y_pos_line.slope, y_pos_line.intercept, T);
					z_position += integral::linearIntegral(z_pos_line.slope, z_pos_line.intercept, T);

					std::println("Velocity  x: {:8.4f}  y: {:8.4f}  z: {:8.4f}", x_velocity,
						y_velocity, z_velocity);
					std::println("Position  x: {:8.4f}  y: {:8.4f}  z: {:8.4f}", x_position,
						y_position, z_position);

					fflush(stdout);
				}

				position_timer = imuData.timestamp;
				samples_since_position = 0;
			}

			//  update rate every second

			if ((now - rateTimer) > rate_update_interval) {
				// sampleRate = sampleCount;
				// sampleCount = 0;
				rateTimer = now;
			}
		}
	}
}
