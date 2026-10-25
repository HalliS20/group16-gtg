#include "Accel.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "utils/Integral.h"
#include "utils/LinearRegression.h"

using std::abs;
using std::size_t;

static constexpr double G = 9.80665;
// all acceleration points below 0.05 are considered noice replaced with 0
static constexpr double accel_deadband = 0.05;
// all velocity below 0.02 is considered noice and replaced with 0
static constexpr double velocity_deadband = 0.02; // m/s
// 13 good window size for balance between noise filtering and drift resistance
static constexpr size_t velocity_window_samples = 13;
// smallest amount of samples possible for working integration
static constexpr size_t stationary_sample_count = 25;

struct Fit {
	bool valid = false;
	LineFit x, y, z;
	double t1 = 0.0, dt = 0.0;
};

static bool allZero(const std::vector<double>& v)
{
	return v.size() == stationary_sample_count && std::ranges::all_of(v, [](double d) {
		return d == 0.0;
	});
}

static void addAccel(Accel& accel, double x, double y, double z, uint64_t ts)
{
	double x_residual = x;
	double y_residual = y;
	double z_residual = z;

	// clearing out acceleration noise
	if (abs(x_residual) < accel_deadband) {
		x_residual = 0;
	}
	if (abs(y_residual) < accel_deadband) {
		y_residual = 0;
	}
	if (abs(z_residual) < accel_deadband) {
		z_residual = 0;
	}

	accel.x_accel_buf.add(x_residual);
	accel.y_accel_buf.add(y_residual);
	accel.z_accel_buf.add(z_residual);
	accel.accel_ts.add(ts);
}

static bool isStationary(const Accel& accel)
{
	return allZero(accel.x_accel_buf.get(stationary_sample_count))
		&& allZero(accel.y_accel_buf.get(stationary_sample_count))
		&& allZero(accel.z_accel_buf.get(stationary_sample_count));
}

// Resets velocity and position on confirmed rest, so drift can't accumulate across movements.
static void zeroMotion(Accel& accel)
{
	accel.x_vel = 0.0;
	accel.y_vel = 0.0;
	accel.z_vel = 0.0;
	accel.x_pos = 0.0;
	accel.y_pos = 0.0;
	accel.z_pos = 0.0;
}

// Fits a line to each axis's trailing accel window. Invalid if there isn't enough data yet.
static Fit fitAccelWindow(const Accel& accel)
{
	Fit fit;
	const std::vector<uint64_t> window_ts = accel.accel_ts.get(velocity_window_samples);
	if (window_ts.size() < 2) {
		return fit;
	}

	const double T = static_cast<double>(window_ts.back() - window_ts.front()) * 1e-6;
	fit.dt = static_cast<double>(window_ts.back() - window_ts[window_ts.size() - 2]) * 1e-6;
	fit.t1 = T - fit.dt;

	fit.x = fitLine(accel.x_accel_buf.get(window_ts.size()), window_ts);
	fit.y = fitLine(accel.y_accel_buf.get(window_ts.size()), window_ts);
	fit.z = fitLine(accel.z_accel_buf.get(window_ts.size()), window_ts);
	fit.valid = fit.x.valid && fit.y.valid && fit.z.valid;
	return fit;
}

// Advances position. Velocity below the deadband doesn't count, returns total distance moved.
static double updatePosition(Accel& accel, const Fit& fit)
{
	const double x_vel_for_pos = std::abs(accel.x_vel) < velocity_deadband ? 0.0 : accel.x_vel;
	const double y_vel_for_pos = std::abs(accel.y_vel) < velocity_deadband ? 0.0 : accel.y_vel;
	const double z_vel_for_pos = std::abs(accel.z_vel) < velocity_deadband ? 0.0 : accel.z_vel;

	const double dx
		= positionStep(fit.x.slope * G, fit.x.intercept * G, fit.t1, fit.dt, x_vel_for_pos);
	const double dy
		= positionStep(fit.y.slope * G, fit.y.intercept * G, fit.t1, fit.dt, y_vel_for_pos);
	const double dz
		= positionStep(fit.z.slope * G, fit.z.intercept * G, fit.t1, fit.dt, z_vel_for_pos);

	accel.x_pos += dx;
	accel.y_pos += dy;
	accel.z_pos += dz;
	return std::sqrt((dx * dx) + (dy * dy) + (dz * dz));
}

// Advances velocity. Call after updatePosition().
static void updateVelocity(Accel& accel, const Fit& fit)
{
	accel.x_vel += velocityStep(fit.x.slope * G, fit.x.intercept * G, fit.t1, fit.dt);
	accel.y_vel += velocityStep(fit.y.slope * G, fit.y.intercept * G, fit.t1, fit.dt);
	accel.z_vel += velocityStep(fit.z.slope * G, fit.z.intercept * G, fit.t1, fit.dt);
}

void addSample(Accel& accel, double x, double y, double z, uint64_t ts)
{
	addAccel(accel, x, y, z, ts);

	if (isStationary(accel)) {
		zeroMotion(accel);
	}

	const Fit fit = fitAccelWindow(accel);
	if (!fit.valid) {
		accel.last_step_distance_m = 0.0;
		return;
	}

	accel.last_step_distance_m = updatePosition(accel, fit);
	updateVelocity(accel, fit);
	accel.has_estimate = true;
}
