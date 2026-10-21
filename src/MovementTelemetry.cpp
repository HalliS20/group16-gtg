#include <cstring>
#include <iostream>
#include <memory>

#include "Logger.hpp"
#include "RTIMULib.h"
#include "circular_buffer.hpp"
#include "integral.hpp"
#include "linear_regression.hpp"

#include "MovementTelemetry.h"

static constexpr const char* MODULE_NAME = "MovementTelemetry";

void MovementTelemetry::sensorProcessingTask()
{
	using namespace Logger;
	using namespace cb;
	using namespace lr;
	using namespace integral;

	constexpr int buffer_length = 1000;

	const double G = 9.80665;
	const double accel_deadband = 0.05;

	CircularBuffer<double> x_accel_buf(buffer_length);
	CircularBuffer<double> y_accel_buf(buffer_length);
	CircularBuffer<double> z_accel_buf(buffer_length);

	CircularBuffer<double> x_vel_buf(buffer_length);
	CircularBuffer<double> y_vel_buf(buffer_length);
	CircularBuffer<double> z_vel_buf(buffer_length);

	CircularBuffer<uint64_t> accel_ts(buffer_length);
	CircularBuffer<uint64_t> vel_ts(buffer_length);

	double x_velocity = 0;
	double y_velocity = 0;
	double z_velocity = 0;
	double x_position = 0;
	double y_position = 0;
	double z_position = 0;

	double x_last_position = 0.0;
	double y_last_position = 0.0;
	double z_last_position = 0.0;

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

	LineFit x_accel_line;
	LineFit y_accel_line;
	LineFit z_accel_line;
	LineFit x_pos_line;
	LineFit y_pos_line;
	LineFit z_pos_line;

	while (true) {
		std::this_thread::sleep_for(std::chrono::microseconds(m_imu->IMUGetPollInterval() * 1000));

		while (m_imu->IMURead()) {

			RTIMU_DATA imuData = m_imu->getIMUData();

			if (imuData.accelValid) {
				RTVector3 linearAccel = m_imu->getAccelResiduals();

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

					x_pos_line = fitLine(x_vel_buf.get(window_ts.size()), window_ts);
					y_pos_line = fitLine(y_vel_buf.get(window_ts.size()), window_ts);
					z_pos_line = fitLine(z_vel_buf.get(window_ts.size()), window_ts);

					// Integral of velocity (m/s) over T seconds gives position in metres metres
					x_position += linearIntegral(x_pos_line.slope, x_pos_line.intercept, T);
					y_position += linearIntegral(y_pos_line.slope, y_pos_line.intercept, T);
					z_position += linearIntegral(z_pos_line.slope, z_pos_line.intercept, T);

					double movement = std::hypot(x_position - x_last_position,
						y_position - y_last_position, z_position - z_last_position);

					m_totalMovement_m.fetch_add(movement);

					x_last_position = x_position;
					y_last_position = y_position;
					z_last_position = z_position;

					// std::cout << x_position << " " << y_position << " " << z_position << std::endl;
					// std::cout << x_velocity << " " << y_velocity << " " << z_velocity << std::endl;
				}

				position_timer = imuData.timestamp;
				samples_since_position = 0;
			}
		}
	}
}

bool MovementTelemetry::init()
{
	using namespace Logger;

	logInfo(MODULE_NAME, "Loading RTIMULib.ini settings.");
	m_imuSettings = std::make_unique<RTIMUSettings>("RTIMULib");

	logInfo(MODULE_NAME, "Detecting the IMU type.");
	m_imu = std::unique_ptr<RTIMU>(RTIMU::createIMU(m_imuSettings.get()));

	if ((nullptr == m_imu) || (m_imu->IMUType() == RTIMU_TYPE_NULL)) {
		logError(MODULE_NAME, "No IMU found!");
		return false;
	}

	logInfo(MODULE_NAME, "Initializing the IMU.");
	if (!m_imu->IMUInit()) {
		logError(MODULE_NAME, "Failed to initialize the IMU!");
		return false;
	}

	logInfo(MODULE_NAME, "Configuring the IMU.");
	m_imu->setSlerpPower(0.02);

	m_imu->setGyroEnable(true);
	m_imu->setAccelEnable(true);
	m_imu->setCompassEnable(true);

	return true;
};

void MovementTelemetry::startRecording()
{
	using namespace Logger;

	logInfo(MODULE_NAME, "Starting the sensor processing thread.");
	m_sensorProcessingThread = std::thread([this]() {
		this->sensorProcessingTask();
	});
}

void MovementTelemetry::stopRecording()
{
	using namespace Logger;

	logInfo(MODULE_NAME, "Stopping the sensor processing thread.");

	if (m_sensorProcessingThread.joinable()) {
		m_sensorProcessingThread.join();
	}
}

[[nodiscard]] double MovementTelemetry::getTotalMovement_m() const
{
	return m_totalMovement_m.load();
}

void MovementTelemetry::resetData()
{
	m_totalMovement_m.store(0.0);
}
