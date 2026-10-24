#ifndef ENVIRONMENT_RUNNER_H
#define ENVIRONMENT_RUNNER_H

#include <atomic>
#include <chrono>
#include <cstddef>

#include "EnvironmentTelemetry.h"

class EnvironmentRunner {
public:
	explicit EnvironmentRunner(EnvironmentTelemetry& telemetry);

	void run(std::atomic<bool>& running);

	[[nodiscard]] bool isInsideTooLong() const;

private:
	EnvironmentTelemetry& m_telemetry;
	std::atomic<bool> m_inside_too_long {false};

	std::size_t m_sample_count = 0;
	std::chrono::steady_clock::time_point m_next_check {};

	void printReading(const EnvironmentTelemetry::Reading& reading);
	void evaluateInsideTooLong(double temp, double humidity);
};

#endif
