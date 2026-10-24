#include <algorithm>
#include <cmath>
#include <cstdint>
#include <print>
#include <unistd.h>
#include <vector>

#include "math/circular_buffer.h"
#include "math/integral.h"
#include "math/linear_regression.h"
#include "utils/Logger.hpp"

#include "MovementTelemetry.h"

static constexpr const char* MODULE_NAME = "MovementTelemetry";

namespace {

constexpr int buffer_length = 25;
constexpr double G = 9.80665;
constexpr double accel_deadband = 0.05;
constexpr double velocity_deadband = 0.02; // m/s

constexpr std::size_t velocity_window_samples = 13;

constexpr std::size_t print_sample_interval = 100;

constexpr std::size_t stationary_sample_count = 25;

constexpr int poll_interval_ms
	= 1; // tighter than IMUGetPollInterval()'s default, steadies the sample rate

struct accel {
	cb::CircularBuffer<double> x_accel_buf = cb::CircularBuffer<double>(buffer_length);
	cb::CircularBuffer<double> y_accel_buf = cb::CircularBuffer<double>(buffer_length);
	cb::CircularBuffer<double> z_accel_buf = cb::CircularBuffer<double>(buffer_length);
	cb::CircularBuffer<uint64_t> accel_ts = cb::CircularBuffer<uint64_t>(buffer_length);

	double x_vel = 0.0, y_vel = 0.0, z_vel = 0.0;
	double x_pos = 0.0, y_pos = 0.0, z_pos = 0.0;
	bool has_estimate = false;
};

bool allZero(const std::vector<double>& v)
{
	return v.size() == stationary_sample_count && std::ranges::all_of(v, [](double d) {
		return d == 0.0;
	});
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

// Resets velocity and position on confirmed rest, so drift can't accumulate across movements.
void zeroMotion(accel& accInstance)
{
	accInstance.x_vel = 0.0;
	accInstance.y_vel = 0.0;
	accInstance.z_vel = 0.0;
	accInstance.x_pos = 0.0;
	accInstance.y_pos = 0.0;
	accInstance.z_pos = 0.0;
}

// A per-axis accel line fit plus the window timing it was fit over.
struct AccelFit {
	bool valid = false;
	lr::LineFit x, y, z;
	double t1 = 0.0, dt = 0.0;
};

// Fits a line to each axis's trailing accel window. Invalid if there isn't enough data yet.
AccelFit fitAccelWindow(const accel& accInstance)
{
	AccelFit fit;
	const std::vector<uint64_t> window_ts = accInstance.accel_ts.get(velocity_window_samples);
	if (window_ts.size() < 2) {
		return fit;
	}

	const double T = static_cast<double>(window_ts.back() - window_ts.front()) * 1e-6;
	fit.dt = static_cast<double>(window_ts.back() - window_ts[window_ts.size() - 2]) * 1e-6;
	fit.t1 = T - fit.dt;

	fit.x = lr::fitLine(accInstance.x_accel_buf.get(window_ts.size()), window_ts);
	fit.y = lr::fitLine(accInstance.y_accel_buf.get(window_ts.size()), window_ts);
	fit.z = lr::fitLine(accInstance.z_accel_buf.get(window_ts.size()), window_ts);
	fit.valid = fit.x.valid && fit.y.valid && fit.z.valid;
	return fit;
}

// Advances position and feeds the step distance into totalMovement. Velocity below the
// deadband doesn't count toward position; above it, used as-is.
void updatePosition(accel& accInstance, const AccelFit& fit, std::atomic<double>& totalMovement)
{
	const double x_vel_for_pos
		= std::abs(accInstance.x_vel) < velocity_deadband ? 0.0 : accInstance.x_vel;
	const double y_vel_for_pos
		= std::abs(accInstance.y_vel) < velocity_deadband ? 0.0 : accInstance.y_vel;
	const double z_vel_for_pos
		= std::abs(accInstance.z_vel) < velocity_deadband ? 0.0 : accInstance.z_vel;

	const double dx = integral::positionStep(
		fit.x.slope * G, fit.x.intercept * G, fit.t1, fit.dt, x_vel_for_pos);
	const double dy = integral::positionStep(
		fit.y.slope * G, fit.y.intercept * G, fit.t1, fit.dt, y_vel_for_pos);
	const double dz = integral::positionStep(
		fit.z.slope * G, fit.z.intercept * G, fit.t1, fit.dt, z_vel_for_pos);

	accInstance.x_pos += dx;
	accInstance.y_pos += dy;
	accInstance.z_pos += dz;
	totalMovement.fetch_add(std::sqrt((dx * dx) + (dy * dy) + (dz * dz)));
}

// Advances velocity. Call after updatePosition(), which needs the pre-update velocity as vPrev.
void updateVelocity(accel& accInstance, const AccelFit& fit)
{
	accInstance.x_vel += integral::velocityStep(fit.x.slope * G, fit.x.intercept * G, fit.t1, fit.dt);
	accInstance.y_vel += integral::velocityStep(fit.y.slope * G, fit.y.intercept * G, fit.t1, fit.dt);
	accInstance.z_vel += integral::velocityStep(fit.z.slope * G, fit.z.intercept * G, fit.t1, fit.dt);
}

} // namespace

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
}

void MovementTelemetry::sensorProcessingTask()
{
	accel accInstance;
	std::size_t sample_count = 0;

	while (m_running) {
		usleep(poll_interval_ms * 1000);

		while (m_imu->IMURead()) {
			const RTIMU_DATA& imuData = m_imu->getIMUData();

			if (!imuData.accelValid) {
				continue;
			}
			const RTVector3 linearAccel = m_imu->getAccelResiduals();
			addAccel(&accInstance, linearAccel, imuData.timestamp);

			if (isStationary(accInstance)) {
				zeroMotion(accInstance);
			}

			const AccelFit fit = fitAccelWindow(accInstance);
			if (fit.valid) {
				updatePosition(accInstance, fit, m_totalMovement_m);
				updateVelocity(accInstance, fit);
				accInstance.has_estimate = true;
			}

			++sample_count;

			if (sample_count < print_sample_interval || !accInstance.has_estimate) {
				continue;
			}

			sample_count = 0;
			std::println("Velocity  x: {:8.4f}  y: {:8.4f}  z: {:8.4f}", accInstance.x_vel,
				accInstance.y_vel, accInstance.z_vel);
			std::println("Position  x: {:8.4f}  y: {:8.4f}  z: {:8.4f}", accInstance.x_pos,
				accInstance.y_pos, accInstance.z_pos);
			std::println("Total movement: {:8.4f}", m_totalMovement_m.load());
			fflush(stdout);
		}
	}
}

void MovementTelemetry::startRecording()
{
	using namespace Logger;

	logInfo(MODULE_NAME, "Starting the sensor processing thread.");
	m_running = true;
	m_sensorProcessingThread = std::thread([this]() {
		this->sensorProcessingTask();
	});
}

void MovementTelemetry::stopRecording()
{
	using namespace Logger;

	logInfo(MODULE_NAME, "Stopping the sensor processing thread.");
	m_running = false;

	if (m_sensorProcessingThread.joinable()) {
		m_sensorProcessingThread.join();
	}
}

double MovementTelemetry::getTotalMovement_m() const
{
	return m_totalMovement_m.load();
}

void MovementTelemetry::resetData()
{
	m_totalMovement_m.store(0.0);
}
