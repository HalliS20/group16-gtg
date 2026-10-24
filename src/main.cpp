#include <chrono>
#include <thread>

#include "DisplayController.h"
#include "EnvironmentTelemetry.h"
#include "Logger.hpp"
#include "MovementTelemetry.h"

static constexpr const char* MODULE_NAME = "GTGApp";

namespace {

constexpr auto inactivityPeriod = std::chrono::seconds(10);
constexpr auto insideTooLongPeriod = std::chrono::seconds(20);
constexpr auto recheckPeriod = std::chrono::seconds(1);
constexpr double movementThreshold_m = 1.0;
constexpr double temperatureRangeThreshold_degC = 4.0;
constexpr double outdoorHumidityMin_percent = 50.0;
constexpr double outdoorHumidityMax_percent = 90.0;

void redraw(DisplayController& display, bool insideTooLong, bool stationaryTooLong)
{
	if (insideTooLong && stationaryTooLong) {
		display.setRed();
	} else if (insideTooLong) {
		display.setGreen();
	} else if (stationaryTooLong) {
		display.setBlue();
	} else {
		display.setWhite();
	}
}

void checkStationary(MovementTelemetry& moveTelemetry, DisplayController& display,
	bool& stationaryTooLong, bool insideTooLong, std::chrono::steady_clock::time_point now,
	std::chrono::steady_clock::time_point& nextCheck)
{
	if (moveTelemetry.getTotalMovement_m() >= movementThreshold_m) {
		moveTelemetry.resetData();
		stationaryTooLong = false;
		nextCheck = now + inactivityPeriod;

		Logger::logInfo(MODULE_NAME, "Movement detected, accumulated value is ok.");
	} else {
		stationaryTooLong = true;
		nextCheck = now + recheckPeriod;

		Logger::logWarning(MODULE_NAME, "Inactivity detected: accumulated movement too low.");
	}
	redraw(display, insideTooLong, stationaryTooLong);
}

void checkInside(EnvironmentTelemetry& envTelemetry, DisplayController& display,
	bool& insideTooLong, bool stationaryTooLong, std::chrono::steady_clock::time_point now,
	std::chrono::steady_clock::time_point& nextCheck)
{
	if (!envTelemetry.hasEstimate()) {
		nextCheck = now + recheckPeriod;
		return;
	}

	const double temperatureRange_degC
		= envTelemetry.getMaxTemperature_degC() - envTelemetry.getMinTemperature_degC();
	const bool sawOutdoorHumidity = envTelemetry.getMinHumidity_percent() <= outdoorHumidityMax_percent
		&& envTelemetry.getMaxHumidity_percent() >= outdoorHumidityMin_percent;

	if (temperatureRange_degC >= temperatureRangeThreshold_degC || sawOutdoorHumidity) {
		envTelemetry.resetData();
		insideTooLong = false;
		nextCheck = now + insideTooLongPeriod;

		Logger::logInfo(MODULE_NAME, "Environment change detected, looks like you've been outside.");
	} else {
		insideTooLong = true;
		nextCheck = now + recheckPeriod;

		Logger::logWarning(MODULE_NAME, "You might not have gone outside in a while.");
	}
	redraw(display, insideTooLong, stationaryTooLong);
}

} // namespace

int main()
{
	DisplayController display {};
	if (!display.init()) {
		Logger::logWarning(MODULE_NAME,
			"Failed to initialize DisplayController module, continuing without display.");
	}

	MovementTelemetry moveTelemetry {};
	if (!moveTelemetry.init()) {
		Logger::logError(MODULE_NAME, "Failed to initialize MovementTelemetry module, exiting.");
		return 1;
	}

	EnvironmentTelemetry envTelemetry {};
	if (!envTelemetry.init()) {
		Logger::logError(MODULE_NAME, "Failed to initialize EnvironmentTelemetry module, exiting.");
		return 1;
	}

	moveTelemetry.startRecording();
	envTelemetry.startRecording();

	bool insideTooLong = false;
	bool stationaryTooLong = false;

	auto nextStationaryCheck = std::chrono::steady_clock::now() + inactivityPeriod;
	auto nextInsideCheck = std::chrono::steady_clock::now() + insideTooLongPeriod;

	while (true) {
		const auto now = std::chrono::steady_clock::now();

		if (now >= nextStationaryCheck) {
			checkStationary(
				moveTelemetry, display, stationaryTooLong, insideTooLong, now, nextStationaryCheck);
		}

		if (now >= nextInsideCheck) {
			checkInside(
				envTelemetry, display, insideTooLong, stationaryTooLong, now, nextInsideCheck);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
