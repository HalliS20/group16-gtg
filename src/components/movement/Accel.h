#ifndef ACCEL_H
#define ACCEL_H

#include <cstdint>

#include "math/circular_buffer.h"

// 3-axis accelerometer dead-reckoning state. Plain data; MovementTelemetry owns and drives it
// via addSample() below.
struct Accel {
	static constexpr int buffer_length = 25;

	cb::CircularBuffer<double> x_accel_buf {buffer_length};
	cb::CircularBuffer<double> y_accel_buf {buffer_length};
	cb::CircularBuffer<double> z_accel_buf {buffer_length};
	cb::CircularBuffer<uint64_t> accel_ts {buffer_length};

	double x_vel = 0.0;
	double y_vel = 0.0;
	double z_vel = 0.0;
	double x_pos = 0.0;
	double y_pos = 0.0;
	double z_pos = 0.0;
	double last_step_distance_m = 0.0;
	bool has_estimate = false;
};

// Feeds in one raw accel sample (m/s^2, gravity removed) and updates position/velocity.
// Sets accel.last_step_distance_m to the distance moved this call, or 0.0 if there wasn't
// enough data yet.
void addSample(Accel& accel, double x, double y, double z, uint64_t ts);

#endif
