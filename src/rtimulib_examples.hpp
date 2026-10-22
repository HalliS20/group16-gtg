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

constexpr std::size_t velocity_window_samples = 10;

constexpr std::size_t print_sample_interval = 100;

constexpr std::size_t stationary_sample_count = 25;

constexpr int poll_interval_ms = 1; // tighter than IMUGetPollInterval()'s default, steadies the sample rate

struct accel {
	cb::CircularBuffer<double> x_accel_buf = cb::CircularBuffer<double>(buffer_length);
	cb::CircularBuffer<double> y_accel_buf = cb::CircularBuffer<double>(buffer_length);
	cb::CircularBuffer<double> z_accel_buf = cb::CircularBuffer<double>(buffer_length);
	cb::CircularBuffer<uint64_t> accel_ts = cb::CircularBuffer<uint64_t>(buffer_length);

	double x_vel = 0.0, y_vel = 0.0, z_vel = 0.0;
	double x_pos = 0.0, y_pos = 0.0, z_pos = 0.0;
	bool has_estimate = false;
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

void zeroVelocity(accel& accInstance)
{
	accInstance.x_vel = 0.0;
	accInstance.y_vel = 0.0;
	accInstance.z_vel = 0.0;
}

// Double-integrates the accel window's line fit directly into velocity and position.
bool updateVelocityAndPosition(accel& accInstance)
{
	const std::vector<uint64_t> window_ts = accInstance.accel_ts.get(velocity_window_samples);
	if (window_ts.size() < 2) {
		return false;
	}

	const double T = static_cast<double>(window_ts.back() - window_ts.front()) * 1e-6;
	const double dt = static_cast<double>(window_ts.back() - window_ts[window_ts.size() - 2]) * 1e-6;
	const double t1 = T - dt;

	const lr::LineFit x_line = lr::fitLine(accInstance.x_accel_buf.get(window_ts.size()), window_ts);
	const lr::LineFit y_line = lr::fitLine(accInstance.y_accel_buf.get(window_ts.size()), window_ts);
	const lr::LineFit z_line = lr::fitLine(accInstance.z_accel_buf.get(window_ts.size()), window_ts);
	if (!x_line.valid || !y_line.valid || !z_line.valid) {
		return false;
	}

	accInstance.x_pos += integral::positionStep(x_line.slope * G, x_line.intercept * G, t1, dt, accInstance.x_vel);
	accInstance.y_pos += integral::positionStep(y_line.slope * G, y_line.intercept * G, t1, dt, accInstance.y_vel);
	accInstance.z_pos += integral::positionStep(z_line.slope * G, z_line.intercept * G, t1, dt, accInstance.z_vel);

	accInstance.x_vel += integral::velocityStep(x_line.slope * G, x_line.intercept * G, t1, dt);
	accInstance.y_vel += integral::velocityStep(y_line.slope * G, y_line.intercept * G, t1, dt);
	accInstance.z_vel += integral::velocityStep(z_line.slope * G, z_line.intercept * G, t1, dt);

	accInstance.has_estimate = true;
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

	std::size_t sample_count = 0;
	while (true) {

		usleep(poll_interval_ms * 1000);

		while (imu->IMURead()) {

			const RTIMU_DATA& imuData = imu->getIMUData();

			if (!imuData.accelValid) {
				continue;
			}
			RTVector3 linearAccel = imu->getAccelResiduals();
			addAccel(&accInstance, linearAccel, imuData.timestamp);

			if (isStationary(accInstance)) {
				zeroVelocity(accInstance);
			}

			updateVelocityAndPosition(accInstance);

			++sample_count;

			if (sample_count < print_sample_interval || !accInstance.has_estimate) {
				continue;
			}

			sample_count = 0;
			std::println("Velocity  x: {:8.4f}  y: {:8.4f}  z: {:8.4f}", accInstance.x_vel, accInstance.y_vel,
				accInstance.z_vel);
			std::println("Position  x: {:8.4f}  y: {:8.4f}  z: {:8.4f}", accInstance.x_pos, accInstance.y_pos,
				accInstance.z_pos);
			fflush(stdout);
		}
	}
}
