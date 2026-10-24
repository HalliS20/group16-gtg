#include "EnvironmentTelemetry.h"

#include "utils/Logger.h"

using Logger::logError;
using Logger::logInfo;
static constexpr const char* MODULE_NAME = "EnvironmentTelemetry";

bool EnvironmentTelemetry::init()
{
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

EnvironmentTelemetry::Reading EnvironmentTelemetry::getReading()
{
	RTIMU_DATA data;
	if (m_humiditySensor->humidityRead(data)) {
		if (data.temperatureValid) {
			m_lastTemperature_degC = data.temperature;
		}
		if (data.humidityValid) {
			m_lastHumidity_percent = data.humidity;
		}
	}

	return {.temperature_degC = m_lastTemperature_degC, .humidity_percent = m_lastHumidity_percent};
}
