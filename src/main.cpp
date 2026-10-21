#include <iostream>

#include "Logger.hpp"
#include "MovementTelemetry.h"
#include "Timer.h"

static constexpr const char* MODULE_NAME = "GTGApp";

int main()
{
	MovementTelemetry moveTelemetry {};
	if (!moveTelemetry.init()) {
		Logger::logError(MODULE_NAME, "Failed to initialize MovementTelemetry module, exiting.");
		return 1;
	}

	constexpr const int inactivityPeriod_s = 10;
	constexpr const int recheckPeriod_s = 1;
	constexpr const double movementThreshold_m = 1.0;

	Timer inactivityTimer(inactivityPeriod_s * 1000);

	inactivityTimer.setCallback([&]() {
		inactivityTimer.setInterval(recheckPeriod_s * 1000);

		if (moveTelemetry.getTotalMovement_m() >= movementThreshold_m) {
			moveTelemetry.resetData();

			inactivityTimer.setInterval(inactivityPeriod_s * 1000);

			std::cout << "NOTIFICATION: the accumulated value is ok" << std::endl;
		} else {
			std::cout << "NOTIFICATION: THE ACCUMULATED VALUE IS TOO LOW" << std::endl;
		}
	});

	moveTelemetry.startRecording();

	inactivityTimer.start();

	while (true) {
		std::cout << "Running the main loop..." << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
}
