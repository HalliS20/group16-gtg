#include <cstdio>
#include <limits>
#include <print>
#include <unistd.h>

#include "Logger.hpp"

#include "EnvironmentTelemetry.h"

static constexpr const char* MODULE_NAME = "EnvironmentTelemetry";

namespace {

// Temperature/humidity drift slowly; no need to poll anywhere near IMU rates.
constexpr int poll_interval_ms = 1000;

// Printed every Nth sample, i.e. roughly every print_sample_interval seconds.
constexpr std::size_t print_sample_interval = 5;

void updateMin(std::atomic<double>& target, double value)
{
	double current = target.load();
	while (value < current && !target.compare_exchange_weak(current, value)) { }
}

void updateMax(std::atomic<double>& target, double value)
{
	double current = target.load();
	while (value > current && !target.compare_exchange_weak(current, value)) { }
}

} // namespace

bool EnvironmentTelemetry::init()
{
	using namespace Logger;

	logInfo(MODULE_NAME, "Loading RTIMULib.ini settings.");
	m_settings = std::make_unique<RTIMUSettings>("RTIMULib");

	logInfo(MODULE_NAME, "Detecting the humidity sensor.");
	m_humiditySensor = std::unique_ptr<RTHumidity>(RTHumidity::createHumidity(m_settings.get()));

	if (nullptr == m_humiditySensor) {
		logError(MODULE_NAME, "No humidity sensor found!");
		return false;
	}

	logInfo(MODULE_NAME, "Initializing the humidity sensor.");
	if (!m_humiditySensor->humidityInit()) {
		logError(MODULE_NAME, "Failed to initialize the humidity sensor!");
		return false;
	}

	return true;
}

void EnvironmentTelemetry::sensorProcessingTask()
{
	RTIMU_DATA data;
	std::size_t sample_count = 0;

	while (m_running) {
		usleep(poll_interval_ms * 1000);

		if (!m_humiditySensor->humidityRead(data)) {
			continue;
		}

		if (data.temperatureValid) {
			updateMin(m_minTemperature_degC, data.temperature);
			updateMax(m_maxTemperature_degC, data.temperature);
			m_hasEstimate = true;
		}
		if (data.humidityValid) {
			updateMin(m_minHumidity_percent, data.humidity);
			updateMax(m_maxHumidity_percent, data.humidity);
			m_hasEstimate = true;
		}

		++sample_count;

		if (sample_count < print_sample_interval || !m_hasEstimate) {
			continue;
		}

		sample_count = 0;
		std::println("Temperature  now: {:6.2f}  range: {:6.2f} - {:6.2f}", data.temperature,
			m_minTemperature_degC.load(), m_maxTemperature_degC.load());
		std::println("Humidity     now: {:6.2f}  range: {:6.2f} - {:6.2f}", data.humidity,
			m_minHumidity_percent.load(), m_maxHumidity_percent.load());
		fflush(stdout);
	}
}

void EnvironmentTelemetry::startRecording()
{
	using namespace Logger;

	logInfo(MODULE_NAME, "Starting the sensor processing thread.");
	m_running = true;
	m_sensorProcessingThread = std::thread([this]() {
		this->sensorProcessingTask();
	});
}

void EnvironmentTelemetry::stopRecording()
{
	using namespace Logger;

	logInfo(MODULE_NAME, "Stopping the sensor processing thread.");
	m_running = false;

	if (m_sensorProcessingThread.joinable()) {
		m_sensorProcessingThread.join();
	}
}

double EnvironmentTelemetry::getMinTemperature_degC() const
{
	return m_minTemperature_degC.load();
}

double EnvironmentTelemetry::getMaxTemperature_degC() const
{
	return m_maxTemperature_degC.load();
}

double EnvironmentTelemetry::getMinHumidity_percent() const
{
	return m_minHumidity_percent.load();
}

double EnvironmentTelemetry::getMaxHumidity_percent() const
{
	return m_maxHumidity_percent.load();
}

bool EnvironmentTelemetry::hasEstimate() const
{
	return m_hasEstimate.load();
}

void EnvironmentTelemetry::resetData()
{
	m_minTemperature_degC.store(std::numeric_limits<double>::infinity());
	m_maxTemperature_degC.store(-std::numeric_limits<double>::infinity());
	m_minHumidity_percent.store(std::numeric_limits<double>::infinity());
	m_maxHumidity_percent.store(-std::numeric_limits<double>::infinity());
	m_hasEstimate.store(false);
}
