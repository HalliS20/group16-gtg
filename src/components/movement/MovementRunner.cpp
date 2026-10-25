#include "MovementRunner.h"

#include <cstdio>
#include <print>
#include <unistd.h>

using std::println;

// how often to print to console
static constexpr std::size_t print_sample_interval = 1000;

// tighter than IMUGetPollInterval()'s default, better for fast sample rate
static constexpr int poll_interval_ms = 1;

static void printReading(const MovementTelemetry::Reading& reading, double total_movement)
{
	println("Velocity  x: {:8.4f}  y: {:8.4f}  z: {:8.4f}", reading.x_vel, reading.y_vel,
		reading.z_vel);
	println("Position  x: {:8.4f}  y: {:8.4f}  z: {:8.4f}", reading.x_pos, reading.y_pos,
		reading.z_pos);
	println("Total movement: {:8.4f}", total_movement);
	fflush(stdout);
}

MovementRunner::MovementRunner(MovementTelemetry& telemetry)
	: m_telemetry(telemetry)
{
}

void MovementRunner::run(std::atomic<bool>& running)
{
	std::size_t sample_count = 0;

	while (running) {
		usleep(poll_interval_ms * 1000);

		const MovementTelemetry::Reading reading = m_telemetry.getReading();
		m_total_movement_m.fetch_add(reading.step_distance_m);

		++sample_count;
		if (sample_count < print_sample_interval || !reading.has_estimate) {
			continue;
		}

		sample_count = 0;
		printReading(reading, m_total_movement_m.load());
	}
}

double MovementRunner::getTotalMovement_m() const
{
	return m_total_movement_m.load();
}

void MovementRunner::resetData()
{
	m_total_movement_m.store(0.0);
}
