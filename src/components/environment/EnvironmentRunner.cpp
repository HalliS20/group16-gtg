#include "EnvironmentRunner.h"

#include <cstdio>
#include <print>
#include <unistd.h>

#include "utils/Logger.h"

using std::chrono::seconds;
using std::chrono::steady_clock;

static constexpr const char* MODULE_NAME = "EnvironmentRunner";
static constexpr int poll_interval_ms = 1000;
static constexpr std::size_t print_sample_interval = 5;
static constexpr seconds inside_too_long_period {20};
static constexpr seconds recheck_period {1};
static constexpr double inside_temp_min = 18.0;
static constexpr double inside_temp_max = 25.0;
static constexpr double inside_hum_max = 50.0;

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

void EnvironmentRunner::evaluateInsideTooLong(double temp, double humidity)
{
	const steady_clock::time_point now = steady_clock::now();
	if (now < m_next_check) {
		return;
	}

	const bool inside_temp = temp >= inside_temp_min && temp <= inside_temp_max;
	const bool inside_hum = humidity < inside_hum_max;
	const bool inside = inside_temp && inside_hum;

	if (!inside) {
		m_inside_too_long = false;
		m_next_check = now + inside_too_long_period;

		Logger::logInfo(MODULE_NAME, "Environment change detected, looks like you've been outside.");
	} else {
		m_inside_too_long = true;
		m_next_check = now + recheck_period;

		Logger::logWarning(MODULE_NAME, "You might not have gone outside in a while.");
	}
}

void EnvironmentRunner::run(std::atomic<bool>& running)
{
	m_next_check = std::chrono::steady_clock::now() + inside_too_long_period;

	while (running) {
		usleep(poll_interval_ms * 1000);

		const EnvironmentTelemetry::Reading reading = m_telemetry.getReading();
		printReading(reading);
		evaluateInsideTooLong(reading.temperature_degC, reading.humidity_percent);
	}
}

bool EnvironmentRunner::isInsideTooLong() const
{
	return m_inside_too_long.load();
}
