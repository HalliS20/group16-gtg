#ifndef RUNNER_H
#define RUNNER_H

#include <atomic>
#include <chrono>
#include <thread>

#include "components/display/DisplayController.h"
#include "components/environment/EnvironmentRunner.h"
#include "components/movement/MovementRunner.h"

using std::atomic;
using std::chrono::steady_clock;

class Runner {
public:
	Runner(DisplayController& d, MovementRunner& m, EnvironmentRunner& e)
		: display(d)
		, move_runner(m)
		, env_runner(e)
	{
	}

	void init();
	void update();

private:
	void checkStationary(steady_clock::time_point now);
	void checkInsideTooLong(steady_clock::time_point now);
	void redraw();

	DisplayController& display;
	MovementRunner& move_runner;
	EnvironmentRunner& env_runner;

	bool inside_too_long = false;
	bool stationary_too_long = false;
	std::chrono::seconds outside_streak {0};
	steady_clock::time_point next_stationary_check;
	steady_clock::time_point next_inside_check;

	atomic<bool> move_running {true};
	atomic<bool> env_running {true};

	std::thread move_thread;
	std::thread env_thread;
};

// Deploys the movement/environment runners on their own threads, then polls their state and
// drives the display from the main thread. Never returns.
void runLoop(DisplayController& display, MovementRunner& move_runner, EnvironmentRunner& env_runner);

#endif
