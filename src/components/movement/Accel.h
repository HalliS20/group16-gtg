#ifndef ACCEL_H
#define ACCEL_H

#include <cstdint>

#include "utils/CircularBuffer.h"

// Used for getting a position change from integration on acceleration
struct Accel {
	static constexpr int buffer_length = 25;

	CircularBuffer<double> x_accel_buf {buffer_length};
	CircularBuffer<double> y_accel_buf {buffer_length};
	CircularBuffer<double> z_accel_buf {buffer_length};
	CircularBuffer<uint64_t> accel_ts {buffer_length};

	double x_vel = 0.0;
	double y_vel = 0.0;
	double z_vel = 0.0;
	double x_pos = 0.0;
	double y_pos = 0.0;
	double z_pos = 0.0;
	double last_step_distance_m = 0.0; // 0.0 until enough samples exist for a motion fit
	bool has_estimate = false;
};

// Feeds in one raw accel sample ; x/y/z are axes, not positions.
void addSample(Accel& accel, double x, double y, double z, uint64_t ts);

#endif
