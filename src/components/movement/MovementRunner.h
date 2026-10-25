#ifndef MOVEMENT_RUNNER_H
#define MOVEMENT_RUNNER_H

#include "MovementTelemetry.h"
#include <atomic>

using std::atomic;

class MovementRunner {
public:
	explicit MovementRunner(MovementTelemetry& telemetry);

	void run(atomic<bool>& running);

	void resetData();

	// Total path length traveled (in meters) since the last resetData() call.
	double getTotalMovement_m() const;

private:
	MovementTelemetry& m_telemetry;
	atomic<double> m_total_movement_m {0.0};
};

#endif
