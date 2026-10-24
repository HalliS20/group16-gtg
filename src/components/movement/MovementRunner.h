#ifndef MOVEMENT_RUNNER_H
#define MOVEMENT_RUNNER_H

#include <atomic>

#include "MovementTelemetry.h"

class MovementRunner {
public:
	explicit MovementRunner(MovementTelemetry& telemetry);

	void run(std::atomic<bool>& running);

	void resetData();

	// Total path length traveled (in meters) since the last resetData() call.
	[[nodiscard]] double getTotalMovement_m() const;

private:
	MovementTelemetry& m_telemetry;
	std::atomic<double> m_total_movement_m {0.0};
};

#endif
