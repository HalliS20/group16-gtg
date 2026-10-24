#ifndef ENVIRONMENT_RUNNER_H
#define ENVIRONMENT_RUNNER_H

#include <atomic>
#include <cstddef>

#include "EnvironmentTelemetry.h"

class EnvironmentRunner {
public:
	explicit EnvironmentRunner(EnvironmentTelemetry& telemetry);

	void run(std::atomic<bool>& running);

	// Latest sensor values, refreshed by run(). Safe to call from another thread.
	EnvironmentTelemetry::Reading getEnvironment() const;

private:
	EnvironmentTelemetry& m_telemetry;

	std::atomic<double> m_temperature_degC {0.0};
	std::atomic<double> m_humidity_percent {0.0};

	std::size_t m_sample_count = 0;

	void printReading(const EnvironmentTelemetry::Reading& reading);
};

#endif
