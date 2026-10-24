#ifndef ENVIRONMENT_TELEMETRY_H
#define ENVIRONMENT_TELEMETRY_H

#include <atomic>
#include <limits>
#include <memory>
#include <thread>

#include "RTIMULib.h"

class EnvironmentTelemetry {
public:
	bool init();

	void startRecording();
	void stopRecording();

	void resetData();

	// Min/max readings seen since the last resetData() call.
	[[nodiscard]] double getMinTemperature_degC() const;
	[[nodiscard]] double getMaxTemperature_degC() const;
	[[nodiscard]] double getMinHumidity_percent() const;
	[[nodiscard]] double getMaxHumidity_percent() const;

	[[nodiscard]] bool hasEstimate() const;

private:
	std::atomic<bool> m_running {false};
	std::atomic<bool> m_hasEstimate {false};

	std::atomic<double> m_minTemperature_degC {std::numeric_limits<double>::infinity()};
	std::atomic<double> m_maxTemperature_degC {-std::numeric_limits<double>::infinity()};
	std::atomic<double> m_minHumidity_percent {std::numeric_limits<double>::infinity()};
	std::atomic<double> m_maxHumidity_percent {-std::numeric_limits<double>::infinity()};

	std::thread m_sensorProcessingThread;
	std::unique_ptr<RTHumidity> m_humiditySensor;
	std::unique_ptr<RTIMUSettings> m_settings;

	void sensorProcessingTask();
};

#endif
