#include "Runner.h"

#include "utils/Logger.h"

static constexpr const char* MODULE_NAME = "GTGApp";

static constexpr std::chrono::seconds inactivity_period {10};
static constexpr std::chrono::seconds recheck_period {1};
static constexpr double movement_threshold_m = 10.0;
static constexpr auto loop_period = std::chrono::milliseconds(100);

static constexpr std::chrono::seconds inside_too_long_period {20};
static constexpr std::chrono::seconds inside_recheck_period {1};
static constexpr std::chrono::seconds outside_reset_period {5};
static constexpr double inside_temp_min = 18.0;
static constexpr double inside_temp_max = 27.0;
static constexpr double inside_hum_max = 50.0;

void Runner::checkStationary(std::chrono::steady_clock::time_point now)
{
	if (move_runner.getTotalMovement_m() >= movement_threshold_m) {
		move_runner.resetData();
		stationary_too_long = false;
		next_stationary_check = now + inactivity_period;

		Logger::logInfo(MODULE_NAME, "Movement detected, accumulated value is ok.");
	} else {
		stationary_too_long = true;
		next_stationary_check = now + recheck_period;

		Logger::logWarning(MODULE_NAME, "Inactivity detected: accumulated movement too low.");
	}
}

void Runner::redraw()
{
	display.showPet(inside_too_long, stationary_too_long);
}

void Runner::init()
{
	next_stationary_check = std::chrono::steady_clock::now() + inactivity_period;
	next_inside_check = std::chrono::steady_clock::now() + inside_too_long_period;

	move_thread = std::thread([this]() {
		move_runner.run(move_running);
	});
	env_thread = std::thread([this]() {
		env_runner.run(env_running);
	});
}

void Runner::checkInsideTooLong(std::chrono::steady_clock::time_point now)
{
	if (now < next_inside_check) {
		return;
	}

	const EnvironmentTelemetry::Reading reading = env_runner.getEnvironment();
	const bool inside_temp = reading.temperature_degC >= inside_temp_min
		&& reading.temperature_degC <= inside_temp_max;
	const bool inside_hum = reading.humidity_percent < inside_hum_max;
	const bool looks_inside = inside_temp && inside_hum;

	if (!looks_inside) {
		// Outside: start/continue the outside streak.
		if (!inside_too_long) {
			inside_too_long = true;
			Logger::logInfo(MODULE_NAME, "Outside detected, outside timer started.");
		}
		outside_streak += inside_recheck_period;
		if (outside_streak >= outside_reset_period) {
			inside_too_long = false;
			outside_streak = std::chrono::seconds {0};
			Logger::logInfo(MODULE_NAME, "Been outside long enough, inside flag cleared.");
		}
		next_inside_check = now + inside_recheck_period;
	} else {
		// Back inside: outside streak no longer counts.
		outside_streak = std::chrono::seconds {0};
		if (!inside_too_long) {
			inside_too_long = true;
			Logger::logWarning(MODULE_NAME, "You might not have gone outside in a while.");
		}
		next_inside_check = now + inside_recheck_period;
	}
}

void Runner::update()
{
	const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

	bool changed = false;

	const bool was_inside_too_long = inside_too_long;
	checkInsideTooLong(now);
	if (inside_too_long != was_inside_too_long) {
		changed = true;
	}

	if (now >= next_stationary_check) {
		checkStationary(now);
		changed = true;
	}

	if (changed) {
		redraw();
	}
}

void runLoop(DisplayController& display, MovementRunner& move_runner, EnvironmentRunner& env_runner)
{
	Runner runner {display, move_runner, env_runner};
	runner.init();

	while (true) {
		runner.update();
		std::this_thread::sleep_for(loop_period);
	}
}
