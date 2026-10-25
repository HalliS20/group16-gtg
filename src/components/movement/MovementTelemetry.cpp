#include "MovementTelemetry.h"

#include "utils/Logger.h"

// Module name is printet to console
static constexpr const char* MODULE_NAME = "MovementTelemetry";
using Logger::logError;
using Logger::logInfo;

bool MovementTelemetry::init()
{

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

MovementTelemetry::Reading MovementTelemetry::getReading()
{
	double step_distance_m = 0.0;

	while (m_imu->IMURead()) {
		const RTIMU_DATA& imu_data = m_imu->getIMUData();
		if (!imu_data.accelValid) {
			continue;
		}

		const RTVector3 linear_accel = m_imu->getAccelResiduals();
		addSample(m_accel, linear_accel.x(), linear_accel.y(), linear_accel.z(), imu_data.timestamp);
		step_distance_m += m_accel.last_step_distance_m;
	}

	return {.has_estimate = m_accel.has_estimate,
		.x_vel = m_accel.x_vel,
		.y_vel = m_accel.y_vel,
		.z_vel = m_accel.z_vel,
		.x_pos = m_accel.x_pos,
		.y_pos = m_accel.y_pos,
		.z_pos = m_accel.z_pos,
		.step_distance_m = step_distance_m};
}
