#ifndef LINEAR_REGRESSION_H
#define LINEAR_REGRESSION_H

#include <cstdint>
#include <vector>

struct LineFit {
	double slope = 0.0;
	double intercept = 0.0;
	bool valid = false;
};

LineFit fitLine(const std::vector<double>& y, const std::vector<uint64_t>& ts);

#endif
