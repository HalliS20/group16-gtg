#ifndef RUNNER_H
#define RUNNER_H

#include <atomic>
#include <chrono>
#include <thread>

#include "components/display/DisplayController.h"
#include "components/environment/EnvironmentRunner.h"
#include "components/movement/MovementRunner.h"

class Runner {
public:
	Runner(DisplayController& d, MovementRunner& m, EnvironmentRunner& e)
		: display(d)
		, moveRunner(m)
		, envRunner(e)
	{
	}

	void init();
	void update();

private:
	void checkStationary(std::chrono::steady_clock::time_point now);
	void checkInsideTooLong(std::chrono::steady_clock::time_point now);
	void redraw();

	DisplayController& display;
	MovementRunner& moveRunner;
	EnvironmentRunner& envRunner;

	bool insideTooLong = false;
	bool stationaryTooLong = false;
	std::chrono::steady_clock::time_point nextStationaryCheck;
	std::chrono::steady_clock::time_point nextInsideCheck;

	std::atomic<bool> moveRunning {true};
	std::atomic<bool> envRunning {true};

	std::thread moveThread;
	std::thread envThread;
};

// Deploys the movement/environment runners on their own threads, then polls their state and
// drives the display from the main thread. Never returns.
void runLoop(DisplayController& display, MovementRunner& moveRunner, EnvironmentRunner& envRunner);

#endif
