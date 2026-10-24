#ifndef MOVEMENT_TELEMETRY_H
#define MOVEMENT_TELEMETRY_H

#include <memory>

#include "Accel.h"

#include "RTIMULib.h"

class MovementTelemetry {
public:
	struct Reading {
		bool has_estimate = false;
		double x_vel = 0.0;
		double y_vel = 0.0;
		double z_vel = 0.0;
		double x_pos = 0.0;
		double y_pos = 0.0;
		double z_pos = 0.0;
		double step_distance_m = 0.0;
	};

	bool init();

	// Drains all currently buffered IMU samples and returns the latest resulting state.
	Reading getReading();

private:
	std::unique_ptr<RTIMU> m_imu;
	std::unique_ptr<RTIMUSettings> m_imuSettings;

	Accel m_accel;
};

#endif
