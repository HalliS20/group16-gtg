#include <chrono>
#include <thread>

#include "DisplayController.h"
#include "EnvironmentTelemetry.h"
#include "Logger.hpp"
#include "MovementTelemetry.h"
#include "Timer.h"
#include "ledmatrixdriver_examples.hpp"

static constexpr const char* MODULE_NAME = "GTGApp";

int main()
{
	run_ledmatrixdriver_example1();

	DisplayController display {};
	if (!display.init(SENSEHAT_FB_NAME, SENSEHAT_FB_SIZE, SENSEHAT_FB_BYTES_PER_PX,
			SENSEHAT_FB_WIDTH_PX, SENSEHAT_FB_HEIGHT_PX, SENSEHAT_TEXT_FILE_ASSET_PATH,
			SENSEHAT_IMAGE_FILE_ASSET_PATH)) {
		Logger::logError(MODULE_NAME, "Failed to initialize DisplayController module, exiting.");
		return 1;
	}

	MovementTelemetry moveTelemetry {};
	if (!moveTelemetry.init()) {
		Logger::logError(MODULE_NAME, "Failed to initialize MovementTelemetry module, exiting.");
		return 1;
	}

	constexpr int inactivityPeriod_s = 10;
	constexpr int recheckPeriod_s = 1;
	constexpr double movementThreshold_m = 1.0;

	Timer inactivityTimer(inactivityPeriod_s * 1000);

	inactivityTimer.setCallback([&]() {
		inactivityTimer.setInterval(recheckPeriod_s * 1000);

		if (moveTelemetry.getTotalMovement_m() >= movementThreshold_m) {
			moveTelemetry.resetData();
			inactivityTimer.setInterval(inactivityPeriod_s * 1000);
			display.setStationaryTooLong(false);

			Logger::logInfo(MODULE_NAME, "Movement detected, accumulated value is ok.");
		} else {
			display.setStationaryTooLong(true);

			Logger::logWarning(MODULE_NAME, "Inactivity detected: accumulated movement too low.");
		}
	});

	EnvironmentTelemetry envTelemetry {};
	if (!envTelemetry.init()) {
		Logger::logError(MODULE_NAME, "Failed to initialize EnvironmentTelemetry module, exiting.");
		return 1;
	}

	constexpr double temperatureRangeThreshold_degC = 4.0;
	constexpr double outdoorHumidityMin_percent = 50.0;
	constexpr double outdoorHumidityMax_percent = 90.0;

	Timer outsideTimer(inactivityPeriod_s * 1000);

	outsideTimer.setCallback([&]() {
		outsideTimer.setInterval(recheckPeriod_s * 1000);

		if (!envTelemetry.hasEstimate()) {
			return;
		}

		const double temperatureRange_degC
			= envTelemetry.getMaxTemperature_degC() - envTelemetry.getMinTemperature_degC();
		const bool sawOutdoorHumidity
			= envTelemetry.getMinHumidity_percent() <= outdoorHumidityMax_percent
			&& envTelemetry.getMaxHumidity_percent() >= outdoorHumidityMin_percent;

		if (temperatureRange_degC >= temperatureRangeThreshold_degC || sawOutdoorHumidity) {
			envTelemetry.resetData();
			outsideTimer.setInterval(inactivityPeriod_s * 1000);
			display.setInsideTooLong(false);

			Logger::logInfo(
				MODULE_NAME, "Environment change detected, looks like you've been outside.");
		} else {
			display.setInsideTooLong(true);

			Logger::logWarning(MODULE_NAME, "You might not have gone outside in a while.");
		}
	});

	moveTelemetry.startRecording();
	inactivityTimer.start();

	envTelemetry.startRecording();
	outsideTimer.start();

	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
}
