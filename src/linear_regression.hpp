#pragma once

#include <cstddef>
#include <vector>

namespace lr {

struct LineFit {
	double slope = 0.0;
	double intercept = 0.0;
	bool valid = false;
};

inline LineFit fitLine(const std::vector<double>& y, double dx = 1.0)
{
	LineFit fit;
	const std::size_t count = y.size();
	if (count < 2)
		return fit;

	const double n = static_cast<double>(count);
	double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumXX = 0.0;
	for (std::size_t i = 0; i < count; ++i) {
		const double x = static_cast<double>(i) * dx;
		sumX += x;
		sumY += y[i];
		sumXY += x * y[i];
		sumXX += x * x;
	}

	const double denom = n * sumXX - sumX * sumX;
	if (denom == 0.0)
		return fit;

	fit.slope = (n * sumXY - sumX * sumY) / denom;
	fit.intercept = (sumY - fit.slope * sumX) / n;
	fit.valid = true;
	return fit;
}

} // namespace lr