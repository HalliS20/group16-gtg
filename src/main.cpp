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
	DisplayController display;

	MovementTelemetry move_telemetry {};
	if (!move_telemetry.init()) {
		Logger::logError(MODULE_NAME, "Failed to initialize MovementTelemetry module, exiting.");
		return 1;
	}

	EnvironmentTelemetry env_telemetry {};
	if (!env_telemetry.init()) {
		Logger::logError(MODULE_NAME, "Failed to initialize EnvironmentTelemetry module, exiting.");
		return 1;
	}

	MovementRunner move_runner(move_telemetry);
	EnvironmentRunner env_runner(env_telemetry);

	runLoop(display, move_runner, env_runner);
}
