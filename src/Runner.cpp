#include "Runner.h"

#include "utils/Logger.h"

static constexpr const char* MODULE_NAME = "GTGApp";

static constexpr std::chrono::seconds inactivityPeriod {10};
static constexpr std::chrono::seconds recheckPeriod {1};
static constexpr double movementThreshold_m = 1.0;
static constexpr auto loopPeriod = std::chrono::milliseconds(100);

static constexpr std::chrono::seconds insideTooLongPeriod {20};
static constexpr std::chrono::seconds insideRecheckPeriod {1};
static constexpr double insideTempMin = 18.0;
static constexpr double insideTempMax = 25.0;
static constexpr double insideHumMax = 50.0;

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

static void initRunner(Runner& runner)
{
	runner.nextStationaryCheck = std::chrono::steady_clock::now() + inactivityPeriod;
	runner.nextInsideCheck = std::chrono::steady_clock::now() + insideTooLongPeriod;

	runner.moveThread = std::thread([&]() {
		runner.moveRunner.run(runner.moveRunning);
	});
	runner.envThread = std::thread([&]() {
		runner.envRunner.run(runner.envRunning);
	});
}

static void checkInsideTooLong(Runner& runner, std::chrono::steady_clock::time_point now)
{
	if (now < runner.nextInsideCheck) {
		return;
	}

	const EnvironmentTelemetry::Reading reading = runner.envRunner.getEnvironment();
	const bool insideTemp
		= reading.temperature_degC >= insideTempMin && reading.temperature_degC <= insideTempMax;
	const bool insideHum = reading.humidity_percent < insideHumMax;

	if (!insideTemp || !insideHum) {
		runner.insideTooLong = false;
		runner.nextInsideCheck = now + insideTooLongPeriod;

		Logger::logInfo(MODULE_NAME, "Environment change detected, looks like you've been outside.");
	} else {
		runner.insideTooLong = true;
		runner.nextInsideCheck = now + insideRecheckPeriod;

		Logger::logWarning(MODULE_NAME, "You might not have gone outside in a while.");
	}
}

static void updateRunner(Runner& runner)
{
	const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

	bool changed = false;

	const bool wasInsideTooLong = runner.insideTooLong;
	checkInsideTooLong(runner, now);
	if (runner.insideTooLong != wasInsideTooLong) {
		changed = true;
	}

	if (now >= runner.nextStationaryCheck) {
		checkStationary(runner, now);
		changed = true;
	}

	if (changed) {
		redraw(runner);
	}
}

void runLoop(DisplayController& display, MovementRunner& moveRunner, EnvironmentRunner& envRunner)
{
	Runner runner {display, moveRunner, envRunner};
	initRunner(runner);

	while (true) {
		updateRunner(runner);
		std::this_thread::sleep_for(loopPeriod);
	}
}
