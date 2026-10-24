#ifndef MATH_LINEAR_REGRESSION_H
#define MATH_LINEAR_REGRESSION_H

#include <cstdint>
#include <vector>

namespace lr {

struct LineFit {
	double slope = 0.0;
	double intercept = 0.0;
	bool valid = false;
};

LineFit fitLine(const std::vector<double>& y, const std::vector<uint64_t>& ts);

} // namespace lr

#endif
