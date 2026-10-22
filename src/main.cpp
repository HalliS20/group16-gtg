#include <chrono>
#include <thread>

#include "Logger.hpp"
#include "MovementTelemetry.h"
#include "Timer.h"
#include "ledmatrixdriver_examples.hpp"

static constexpr const char* MODULE_NAME = "GTGApp";

int main()
{
	run_ledmatrixdriver_example1();

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

			Logger::logInfo(MODULE_NAME, "Movement detected, accumulated value is ok.");
		} else {
			Logger::logWarning(MODULE_NAME, "Inactivity detected: accumulated movement too low.");
		}
	});

	moveTelemetry.startRecording();
	inactivityTimer.start();

	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
}
