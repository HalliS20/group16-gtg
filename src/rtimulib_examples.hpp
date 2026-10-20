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
constexpr double G = 9.80665;
constexpr double accel_deadband = 0.05;

constexpr std::size_t velocity_batch_samples = 10;
constexpr std::size_t position_batch_samples = 10;

constexpr std::size_t print_sample_interval = 100;

constexpr std::size_t stationary_sample_count = 25;

struct accel {
	cb::CircularBuffer<double> x_accel_buf = cb::CircularBuffer<double>(buffer_length);
	cb::CircularBuffer<double> y_accel_buf = cb::CircularBuffer<double>(buffer_length);
	cb::CircularBuffer<double> z_accel_buf = cb::CircularBuffer<double>(buffer_length);
	cb::CircularBuffer<uint64_t> accel_ts = cb::CircularBuffer<uint64_t>(buffer_length);

	cb::CircularBuffer<double> x_vel_buf = cb::CircularBuffer<double>(buffer_length);
	cb::CircularBuffer<double> y_vel_buf = cb::CircularBuffer<double>(buffer_length);
	cb::CircularBuffer<double> z_vel_buf = cb::CircularBuffer<double>(buffer_length);
	cb::CircularBuffer<uint64_t> vel_ts = cb::CircularBuffer<uint64_t>(buffer_length);

	cb::CircularBuffer<double> x_pos_buf = cb::CircularBuffer<double>(buffer_length);
	cb::CircularBuffer<double> y_pos_buf = cb::CircularBuffer<double>(buffer_length);
	cb::CircularBuffer<double> z_pos_buf = cb::CircularBuffer<double>(buffer_length);
	cb::CircularBuffer<uint64_t> pos_ts = cb::CircularBuffer<uint64_t>(buffer_length);
};

RTIMU* initIMU()
{
	RTIMUSettings* settings = new RTIMUSettings("RTIMULib");
	RTIMU* imu = RTIMU::createIMU(settings);
	if ((imu == nullptr) || (imu->IMUType() == RTIMU_TYPE_NULL)) {
		std::println("No IMU found");
		exit(1);
	}

	imu->IMUInit();

	imu->setSlerpPower(0.02);
	imu->setGyroEnable(true);
	imu->setAccelEnable(true);
	imu->setCompassEnable(true);
	return imu;
}

bool allZero(const std::vector<double>& v)
{
	if (v.size() != stationary_sample_count) {
		return false;
	}
	for (double d : v) {
		if (d != 0.0) {
			return false;
		}
	}
	return true;
}

bool isStationary(const accel& accInstance)
{
	return allZero(accInstance.x_accel_buf.get(stationary_sample_count))
		&& allZero(accInstance.y_accel_buf.get(stationary_sample_count))
		&& allZero(accInstance.z_accel_buf.get(stationary_sample_count));
}

void addAccel(accel* accInstance, const RTVector3& linearAccel, uint64_t ts)
{
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

	accInstance->x_accel_buf.add(x_residual);
	accInstance->y_accel_buf.add(y_residual);
	accInstance->z_accel_buf.add(z_residual);
	accInstance->accel_ts.add(ts);
}

void zeroVelocity(accel& accInstance, uint64_t ts)
{
	accInstance.x_vel_buf.add(0.0);
	accInstance.y_vel_buf.add(0.0);
	accInstance.z_vel_buf.add(0.0);
	accInstance.vel_ts.add(ts);
}

bool updateVelocity(accel& accInstance, std::size_t window_samples)
{
	const std::vector<uint64_t> window_ts = accInstance.accel_ts.get(window_samples);
	if (window_ts.size() < 2) {
		return false;
	}

	const double T = static_cast<double>(window_ts.back() - window_ts.front()) * 1e-6;

	const lr::LineFit x_line = lr::fitLine(accInstance.x_accel_buf.get(window_ts.size()), window_ts);
	const lr::LineFit y_line = lr::fitLine(accInstance.y_accel_buf.get(window_ts.size()), window_ts);
	const lr::LineFit z_line = lr::fitLine(accInstance.z_accel_buf.get(window_ts.size()), window_ts);
	if (!x_line.valid || !y_line.valid || !z_line.valid) {
		return false;
	}

	const double x_prev = accInstance.x_vel_buf.size() > 0 ? accInstance.x_vel_buf.getLast() : 0.0;
	const double y_prev = accInstance.y_vel_buf.size() > 0 ? accInstance.y_vel_buf.getLast() : 0.0;
	const double z_prev = accInstance.z_vel_buf.size() > 0 ? accInstance.z_vel_buf.getLast() : 0.0;

	accInstance.x_vel_buf.add(x_prev + (integral::linearIntegral(x_line.slope, x_line.intercept, T) * G));
	accInstance.y_vel_buf.add(y_prev + (integral::linearIntegral(y_line.slope, y_line.intercept, T) * G));
	accInstance.z_vel_buf.add(z_prev + (integral::linearIntegral(z_line.slope, z_line.intercept, T) * G));
	accInstance.vel_ts.add(window_ts.back());
	return true;
}

bool updatePosition(accel& accInstance, std::size_t window_samples)
{
	const std::vector<uint64_t> window_ts = accInstance.vel_ts.get(window_samples);
	if (window_ts.size() < 2) {
		return false;
	}

	const double T = static_cast<double>(window_ts.back() - window_ts.front()) * 1e-6;

	const lr::LineFit x_line = lr::fitLine(accInstance.x_vel_buf.get(window_ts.size()), window_ts);
	const lr::LineFit y_line = lr::fitLine(accInstance.y_vel_buf.get(window_ts.size()), window_ts);
	const lr::LineFit z_line = lr::fitLine(accInstance.z_vel_buf.get(window_ts.size()), window_ts);
	if (!x_line.valid || !y_line.valid || !z_line.valid) {
		return false;
	}

	const double x_prev = accInstance.x_pos_buf.size() > 0 ? accInstance.x_pos_buf.getLast() : 0.0;
	const double y_prev = accInstance.y_pos_buf.size() > 0 ? accInstance.y_pos_buf.getLast() : 0.0;
	const double z_prev = accInstance.z_pos_buf.size() > 0 ? accInstance.z_pos_buf.getLast() : 0.0;

	accInstance.x_pos_buf.add(x_prev + integral::linearIntegral(x_line.slope, x_line.intercept, T));
	accInstance.y_pos_buf.add(y_prev + integral::linearIntegral(y_line.slope, y_line.intercept, T));
	accInstance.z_pos_buf.add(z_prev + integral::linearIntegral(z_line.slope, z_line.intercept, T));
	accInstance.pos_ts.add(window_ts.back());
	return true;
}

// RTPressure* pressure = RTPressure::createPressure(settings);
// RTHumidity* humidity = RTHumidity::createHumidity(settings);
// 	if (pressure != NULL) {
// 		pressure->pressureInit();
// 	}
//
// 	if (humidity != NULL) {
// 		humidity->humidityInit();
// 	}

inline int run_rtimulib_example1()
{
	RTIMU* imu = initIMU();

	accel accInstance;

	std::size_t samples_since_velocity = 0;
	std::size_t samples_since_position = 0;

	std::size_t sample_count = 0;
	while (true) {

		usleep(imu->IMUGetPollInterval() * 1000);

		while (imu->IMURead()) {

			const RTIMU_DATA& imuData = imu->getIMUData();

			if (!imuData.accelValid) {
				continue;
			}
			RTVector3 linearAccel = imu->getAccelResiduals();
			addAccel(&accInstance, linearAccel, imuData.timestamp);

			if (isStationary(accInstance)) {
				zeroVelocity(accInstance, imuData.timestamp);
			}

			++samples_since_velocity;
			if (samples_since_velocity >= velocity_batch_samples) {
				if (updateVelocity(accInstance, samples_since_velocity + 1)) {
					++samples_since_position;
				}
				samples_since_velocity = 0;
			}

			if (samples_since_position >= position_batch_samples) {
				updatePosition(accInstance, samples_since_position + 1);
				samples_since_position = 0;
			}

			++sample_count;

			if (sample_count < print_sample_interval || accInstance.x_vel_buf.size() == 0
				|| accInstance.x_pos_buf.size() == 0) {
				continue;
			}

			sample_count = 0;
			const double x_velocity = accInstance.x_vel_buf.getLast();
			const double y_velocity = accInstance.y_vel_buf.getLast();
			const double z_velocity = accInstance.z_vel_buf.getLast();
			const double x_position = accInstance.x_pos_buf.getLast();
			const double y_position = accInstance.y_pos_buf.getLast();
			const double z_position = accInstance.z_pos_buf.getLast();

			std::println(
				"Velocity  x: {:8.4f}  y: {:8.4f}  z: {:8.4f}", x_velocity, y_velocity, z_velocity);
			std::println(
				"Position  x: {:8.4f}  y: {:8.4f}  z: {:8.4f}", x_position, y_position, z_position);
			fflush(stdout);
		}
	}
}
