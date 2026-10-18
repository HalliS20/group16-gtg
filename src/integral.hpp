#pragma once

namespace integral {

inline double linearIntegral(double a, double b, double x1)
{
	double a_term = 0.5 * x1 * x1 * a;
	double b_term = x1 * b;
	return a_term + b_term;
}

} // namespace integral
