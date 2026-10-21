/**
 * @file MovementTelemetry.h
 * @brief
 * @version 0.1
 *
 *
 */
#ifndef _MOVEMENT_TELEMETRY_H
#define _MOVEMENT_TELEMETRY_H

#include <atomic>
#include <memory>
#include <thread>

#include "RTIMULib.h"

class MovementTelemetry {
public:
	/**
	 * @brief
	 *
	 * @return true
	 * @return false
	 */
	bool init();

	/**
	 * @brief
	 *
	 */
	void startRecording();

	/**
	 * @brief
	 *
	 */
	void stopRecording();

	/**
	 * @brief
	 *
	 */
	void resetData();

	/**
	 * @brief Get the Total Movement Meters
	 *
	 * @return double - total movement since recording started, in meters.
	 */
	[[nodiscard]] double getTotalMovement_m() const;

private:
	std::atomic<double> m_totalMovement_m;

	std::thread m_sensorProcessingThread;
	std::unique_ptr<RTIMU> m_imu;
	std::unique_ptr<RTIMUSettings> m_imuSettings;

	void sensorProcessingTask();
};

#endif
