#include "EnvironmentRunner.h"

#include <cstdio>
#include <print>
#include <unistd.h>

// how often to print every 5 samples rn
static constexpr std::size_t print_sample_interval = 5;
// how often to poll the sensor
static constexpr int poll_interval_ms = 1000;

EnvironmentRunner::EnvironmentRunner(EnvironmentTelemetry& telemetry)
	: m_telemetry(telemetry)
{
}

void EnvironmentRunner::printReading(const EnvironmentTelemetry::Reading& reading)
{
	++m_sample_count;
	if (m_sample_count < print_sample_interval) {
		return;
	}
	m_sample_count = 0;

	std::println("Temperature  now: {:6.2f}", reading.temperature_degC);
	std::println("Humidity     now: {:6.2f}", reading.humidity_percent);
	fflush(stdout);
}

EnvironmentTelemetry::Reading EnvironmentRunner::getEnvironment() const
{
	EnvironmentTelemetry::Reading reading;
	reading.temperature_degC = m_temperature_degC.load();
	reading.humidity_percent = m_humidity_percent.load();
	return reading;
}

void EnvironmentRunner::run(std::atomic<bool>& running)
{
	while (running) {
		usleep(poll_interval_ms * 1000);

		const EnvironmentTelemetry::Reading reading = m_telemetry.getReading();
		m_temperature_degC.store(reading.temperature_degC);
		m_humidity_percent.store(reading.humidity_percent);
		printReading(reading);
	}
}
