#include "linear_regression.h"

namespace lr {

LineFit fitLine(const std::vector<double>& y, const std::vector<uint64_t>& ts)
{
	LineFit fit;
	const std::size_t count = y.size();
	if (count < 2 || ts.size() != count) {
		return fit;
	}

	const double n = static_cast<double>(count);
	double sumX = 0.0;
	double sumY = 0.0;
	double sumXY = 0.0;
	double sumXX = 0.0;
	for (std::size_t i = 0; i < count; ++i) {
		const double x = static_cast<double>(ts[i] - ts[0]) * 1e-6;
		sumX += x;
		sumY += y[i];
		sumXY += x * y[i];
		sumXX += x * x;
	}

	const double denom = (n * sumXX) - (sumX * sumX);
	if (denom == 0.0) {
		return fit;
	}

	fit.slope = ((n * sumXY) - (sumX * sumY)) / denom;
	fit.intercept = (sumY - (fit.slope * sumX)) / n;
	fit.valid = true;
	return fit;
}

} // namespace lr
