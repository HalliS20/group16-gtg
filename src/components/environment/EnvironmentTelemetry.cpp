#include "EnvironmentTelemetry.h"

#include "utils/Logger.h"

using Logger::logError;
using Logger::logInfo;
static constexpr const char* MODULE_NAME = "EnvironmentTelemetry";
static constexpr double CPU_HEAT_COMPENSATION_DIVISOR = 1.5;

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

	// Compensate for heat radiating from Pi CPU into the HAT sensor.
	const double cpuTemp = getCpuTemperature_degC();
	const double correctedTemp = m_lastTemperature_degC
		- ((cpuTemp - m_lastTemperature_degC) / CPU_HEAT_COMPENSATION_DIVISOR);

	return {.temperature_degC = correctedTemp, .humidity_percent = m_lastHumidity_percent};
}

double EnvironmentTelemetry::getCpuTemperature_degC()
{
	FILE* fp = std::fopen("/sys/class/thermal/thermal_zone0/temp", "r");
	if (nullptr == fp) {
		return 0.0;
	}

	long milliC = 0;
	const int scanned = std::fscanf(fp, "%ld", &milliC);
	std::fclose(fp);

	if (1 != scanned) {
		return 0.0;
	}
	return static_cast<double>(milliC) / 1000.0;
}
