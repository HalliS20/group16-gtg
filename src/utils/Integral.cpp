#include "Integral.h"

double positionStep(double slope, double intercept, double t1, double dt, double v_prev)
{
	return (v_prev * dt) + ((1.0 / 6.0) * slope * dt * dt * dt)
		+ (0.5 * ((slope * t1) + intercept) * dt * dt);
}

double velocityStep(double slope, double intercept, double t1, double dt)
{
	const double t2 = t1 + dt;
	return (0.5 * slope * ((t2 * t2) - (t1 * t1))) + (intercept * dt);
}
