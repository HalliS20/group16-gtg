#include "Runner.h"
#include "components/display/DisplayController.h"
#include "components/environment/EnvironmentRunner.h"
#include "components/environment/EnvironmentTelemetry.h"
#include "components/movement/MovementRunner.h"
#include "components/movement/MovementTelemetry.h"
#include "utils/Logger.h"

static constexpr const char* MODULE_NAME = "GTGApp";

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

	MovementRunner moveRunner(moveTelemetry);
	EnvironmentRunner envRunner(envTelemetry);

	Runner runner {display, moveRunner, envRunner};
	runLoop(runner);
}
