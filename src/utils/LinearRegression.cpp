#include "LinearRegression.h"

using std::vector;

LineFit fitLine(const vector<double>& y, const vector<uint64_t>& ts)
{
	LineFit fit;
	const std::size_t count = y.size();
	if (count < 2 || ts.size() != count) {
		return fit;
	}

	const double n = static_cast<double>(count);
	double sum_x = 0.0;
	double sum_y = 0.0;
	double sum_xy = 0.0;
	double sum_xx = 0.0;
	for (std::size_t i = 0; i < count; ++i) {
		const double x = static_cast<double>(ts[i] - ts[0]) * 1e-6;
		sum_x += x;
		sum_y += y[i];
		sum_xy += x * y[i];
		sum_xx += x * x;
	}

	const double denom = (n * sum_xx) - (sum_x * sum_x);
	if (denom == 0.0) {
		return fit;
	}

	fit.slope = ((n * sum_xy) - (sum_x * sum_y)) / denom;
	fit.intercept = (sum_y - (fit.slope * sum_x)) / n;
	fit.valid = true;
	return fit;
}
