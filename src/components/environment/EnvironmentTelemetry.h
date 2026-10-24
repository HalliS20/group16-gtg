#ifndef ENVIRONMENT_TELEMETRY_H
#define ENVIRONMENT_TELEMETRY_H

#include <memory>

#include "RTIMULib.h"
#include <cstdio>
#include <string>

class EnvironmentTelemetry {
public:
	struct Reading {
		double temperature_degC = 0.0;
		double humidity_percent = 0.0;
	};

	bool init();

	// Falls back to the last valid value per field if the sensor had nothing new.
	[[nodiscard]] Reading getReading();

private:
	static double getCpuTemp();
	std::unique_ptr<RTHumidity> m_humiditySensor;
	std::unique_ptr<RTIMUSettings> m_settings;

	double m_lastTemperature_degC = 0.0;
	double m_lastHumidity_percent = 0.0;
};

#endif
