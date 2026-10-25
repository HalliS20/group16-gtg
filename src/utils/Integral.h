#ifndef INTEGRAL_H
#define INTEGRAL_H

// Integral to get position from velocity
double positionStep(double slope, double intercept, double t1, double dt, double v_prev);

// Integral to get velocity from acceleration
double velocityStep(double slope, double intercept, double t1, double dt);

#endif
