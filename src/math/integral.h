#ifndef MATH_INTEGRAL_H
#define MATH_INTEGRAL_H

namespace integral {

// Double integral of a(t) = slope*t + intercept over [t1, t1+dt], given the velocity already accumulated up to t1.
double positionStep(double slope, double intercept, double t1, double dt, double vPrev);

// Integral of the same line over [t1, t1+dt].
double velocityStep(double slope, double intercept, double t1, double dt);

} // namespace integral

#endif
