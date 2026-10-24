#ifndef MOVEMENT_TELEMETRY_H
#define MOVEMENT_TELEMETRY_H

#include <atomic>
#include <memory>
#include <thread>

#include "RTIMULib.h"

class MovementTelemetry {
public:
	bool init();

	void startRecording();
	void stopRecording();

	void resetData();

	// Total path length traveled (in meters) since the last resetData() call.
	[[nodiscard]] double getTotalMovement_m() const;

private:
	std::atomic<bool> m_running {false};
	std::atomic<double> m_totalMovement_m {0.0};

	std::thread m_sensorProcessingThread;
	std::unique_ptr<RTIMU> m_imu;
	std::unique_ptr<RTIMUSettings> m_imuSettings;

	void sensorProcessingTask();
};

#endif
