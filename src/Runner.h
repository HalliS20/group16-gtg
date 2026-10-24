#ifndef RUNNER_H
#define RUNNER_H

#include <chrono>

#include "components/display/DisplayController.h"
#include "components/environment/EnvironmentRunner.h"
#include "components/movement/MovementRunner.h"

struct Runner {
	DisplayController& display;
	MovementRunner& moveRunner;
	EnvironmentRunner& envRunner;

	bool insideTooLong = false;
	bool stationaryTooLong = false;
	std::chrono::steady_clock::time_point nextStationaryCheck {};
};

// Deploys the movement/environment runners on their own threads, then polls their state and
// drives the display from the main thread. Never returns.
void runLoop(Runner& runner);

#endif
