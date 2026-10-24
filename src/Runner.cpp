#include "Runner.h"

#include "utils/Logger.h"

static constexpr const char* MODULE_NAME = "GTGApp";

static constexpr std::chrono::seconds inactivityPeriod {10};
static constexpr std::chrono::seconds recheckPeriod {1};
static constexpr double movementThreshold_m = 10.0;
static constexpr auto loopPeriod = std::chrono::milliseconds(100);

static constexpr std::chrono::seconds insideTooLongPeriod {20};
static constexpr std::chrono::seconds insideRecheckPeriod {1};
static constexpr std::chrono::seconds outsideResetPeriod {60};
static constexpr double insideTempMin = 18.0;
static constexpr double insideTempMax = 27.0;
static constexpr double insideHumMax = 50.0;

void Runner::checkStationary(std::chrono::steady_clock::time_point now)
{
	if (moveRunner.getTotalMovement_m() >= movementThreshold_m) {
		moveRunner.resetData();
		stationaryTooLong = false;
		nextStationaryCheck = now + inactivityPeriod;

		Logger::logInfo(MODULE_NAME, "Movement detected, accumulated value is ok.");
	} else {
		stationaryTooLong = true;
		nextStationaryCheck = now + recheckPeriod;

		Logger::logWarning(MODULE_NAME, "Inactivity detected: accumulated movement too low.");
	}
}

void Runner::redraw()
{
	display.showPet(insideTooLong, stationaryTooLong);
}

void Runner::init()
{
	nextStationaryCheck = std::chrono::steady_clock::now() + inactivityPeriod;
	nextInsideCheck = std::chrono::steady_clock::now() + insideTooLongPeriod;

	moveThread = std::thread([this]() {
		moveRunner.run(moveRunning);
	});
	envThread = std::thread([this]() {
		envRunner.run(envRunning);
	});
}

void Runner::checkInsideTooLong(std::chrono::steady_clock::time_point now)
{
	if (now < nextInsideCheck) {
		return;
	}

	const EnvironmentTelemetry::Reading reading = envRunner.getEnvironment();
	const bool insideTemp
		= reading.temperature_degC >= insideTempMin && reading.temperature_degC <= insideTempMax;
	const bool insideHum = reading.humidity_percent < insideHumMax;
	const bool looksInside = insideTemp && insideHum;

	if (!looksInside) {
		// Outside: start/continue the outside streak.
		if (!insideTooLong) {
			insideTooLong = true;
			Logger::logInfo(MODULE_NAME, "Outside detected, outside timer started.");
		}
		outsideStreak += insideRecheckPeriod;
		if (outsideStreak >= outsideResetPeriod) {
			insideTooLong = false;
			outsideStreak = std::chrono::seconds {0};
			Logger::logInfo(MODULE_NAME, "Been outside long enough, inside flag cleared.");
		}
		nextInsideCheck = now + insideRecheckPeriod;
	} else {
		// Back inside: outside streak no longer counts.
		outsideStreak = std::chrono::seconds {0};
		if (!insideTooLong) {
			insideTooLong = true;
			Logger::logWarning(MODULE_NAME, "You might not have gone outside in a while.");
		}
		nextInsideCheck = now + insideRecheckPeriod;
	}
}

void Runner::update()
{
	const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

	bool changed = false;

	const bool wasInsideTooLong = insideTooLong;
	checkInsideTooLong(now);
	if (insideTooLong != wasInsideTooLong) {
		changed = true;
	}

	if (now >= nextStationaryCheck) {
		checkStationary(now);
		changed = true;
	}

	if (changed) {
		redraw();
	}
}

void runLoop(DisplayController& display, MovementRunner& moveRunner, EnvironmentRunner& envRunner)
{
	Runner runner {display, moveRunner, envRunner};
	runner.init();

	while (true) {
		runner.update();
		std::this_thread::sleep_for(loopPeriod);
	}
}
