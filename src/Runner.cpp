#include "Runner.h"

#include <atomic>
#include <thread>

#include "utils/Logger.h"

static constexpr const char* MODULE_NAME = "GTGApp";

static constexpr std::chrono::seconds inactivityPeriod {10};
static constexpr std::chrono::seconds recheckPeriod {1};
static constexpr double movementThreshold_m = 1.0;

static void checkStationary(Runner& runner, std::chrono::steady_clock::time_point now)
{
	if (runner.moveRunner.getTotalMovement_m() >= movementThreshold_m) {
		runner.moveRunner.resetData();
		runner.stationaryTooLong = false;
		runner.nextStationaryCheck = now + inactivityPeriod;

		Logger::logInfo(MODULE_NAME, "Movement detected, accumulated value is ok.");
	} else {
		runner.stationaryTooLong = true;
		runner.nextStationaryCheck = now + recheckPeriod;

		Logger::logWarning(MODULE_NAME, "Inactivity detected: accumulated movement too low.");
	}
}

static void redraw(Runner& runner)
{
	if (runner.insideTooLong && runner.stationaryTooLong) {
		runner.display.setRed();
	} else if (runner.insideTooLong) {
		runner.display.setGreen();
	} else if (runner.stationaryTooLong) {
		runner.display.setBlue();
	} else {
		runner.display.setWhite();
	}
}

void runLoop(Runner& runner)
{
	runner.nextStationaryCheck = std::chrono::steady_clock::now() + inactivityPeriod;

	std::atomic<bool> moveRunning {true};
	std::atomic<bool> envRunning {true};

	const std::thread moveThread([&]() {
		runner.moveRunner.run(moveRunning);
	});
	const std::thread envThread([&]() {
		runner.envRunner.run(envRunning);
	});

	while (true) {
		const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

		bool changed = false;

		const bool newInsideTooLong = runner.envRunner.isInsideTooLong();
		if (newInsideTooLong != runner.insideTooLong) {
			runner.insideTooLong = newInsideTooLong;
			changed = true;
		}

		if (now >= runner.nextStationaryCheck) {
			checkStationary(runner, now);
			changed = true;
		}

		if (changed) {
			redraw(runner);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
