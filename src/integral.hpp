#pragma once

namespace integral {

// Double integral of a(t) = slope*t + intercept over [t1, t1+dt], given the velocity already accumulated up to t1.
inline double positionStep(double slope, double intercept, double t1, double dt, double vPrev)
{
	return (vPrev * dt) + ((1.0 / 6.0) * slope * dt * dt * dt)
		+ (0.5 * ((slope * t1) + intercept) * dt * dt);
}

// Integral of the same line over [t1, t1+dt].
inline double velocityStep(double slope, double intercept, double t1, double dt)
{
	const double t2 = t1 + dt;
	return (0.5 * slope * ((t2 * t2) - (t1 * t1))) + (intercept * dt);
}

} // namespace integral
