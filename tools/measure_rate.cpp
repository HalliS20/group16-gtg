// Standalone diagnostic: measure the IMU's actual achieved accel sample
// rate on real hardware, not just the rate implied by RTIMULib.ini.
// Reports the average rate AND the distribution of inter-sample gaps,
// since the slowest gap (the floor) is what determines whether a brief
// real-world deceleration gets seen at all.
//
// Bounded runtime (default 15s, pass seconds as argv[1]) so it's safe to
// run unattended; does not touch any of the project's velocity/position
// pipeline code.

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <print>
#include <unistd.h>
#include <vector>

#include "RTIMULib.h"

namespace {

bool envFlag(const char* name, bool defaultValue)
{
	const char* v = std::getenv(name);
	if (v == nullptr) {
		return defaultValue;
	}
	return v[0] != '0';
}

RTIMU* initIMU(bool gyroEnable, bool compassEnable)
{
	RTIMUSettings* settings = new RTIMUSettings("RTIMULib");
	RTIMU* imu = RTIMU::createIMU(settings);
	if ((imu == nullptr) || (imu->IMUType() == RTIMU_TYPE_NULL)) {
		std::println("No IMU found");
		exit(1);
	}

	imu->IMUInit();
	imu->setSlerpPower(0.02);
	imu->setGyroEnable(gyroEnable);
	imu->setAccelEnable(true);
	imu->setCompassEnable(compassEnable);
	return imu;
}

} // namespace

int main(int argc, char** argv)
{
	const double runSeconds = argc > 1 ? std::atof(argv[1]) : 15.0;
	const bool gyroEnable = envFlag("GTG_GYRO", true);
	const bool compassEnable = envFlag("GTG_COMPASS", true);
	const char* pollOverride = std::getenv("GTG_POLL_MS");

	RTIMU* imu = initIMU(gyroEnable, compassEnable);
	const int pollMs = pollOverride != nullptr ? std::atoi(pollOverride) : imu->IMUGetPollInterval();
	std::println("configured poll interval: {} ms  (using: {} ms)  gyro={} compass={}", imu->IMUGetPollInterval(),
		pollMs, gyroEnable, compassEnable);

	std::vector<uint64_t> timestamps;
	timestamps.reserve(20000);

	double magSum = 0.0;
	double magMin = 1e9;
	double magMax = 0.0;
	std::size_t magCount = 0;

	const auto start = std::chrono::steady_clock::now();
	while (std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count() < runSeconds) {
		usleep(pollMs * 1000);
		while (imu->IMURead()) {
			const RTIMU_DATA& imuData = imu->getIMUData();
			if (!imuData.accelValid) {
				continue;
			}
			timestamps.push_back(imuData.timestamp);

			const RTVector3& a = imuData.accel;
			const double mag = std::sqrt((a.x() * a.x()) + (a.y() * a.y()) + (a.z() * a.z()));
			magSum += mag;
			magMin = std::min(magMin, mag);
			magMax = std::max(magMax, mag);
			++magCount;
		}
	}

	if (magCount > 0) {
		std::println("raw accel magnitude (should be ~1.0g at rest): mean={:.4f}  min={:.4f}  max={:.4f}",
			magSum / static_cast<double>(magCount), magMin, magMax);
	}

	if (timestamps.size() < 2) {
		std::println("not enough samples captured ({})", timestamps.size());
		return 1;
	}

	std::vector<uint64_t> dts;
	dts.reserve(timestamps.size() - 1);
	for (std::size_t i = 1; i < timestamps.size(); ++i) {
		dts.push_back(timestamps[i] - timestamps[i - 1]);
	}
	std::ranges::sort(dts);

	const double totalSeconds = static_cast<double>(timestamps.back() - timestamps.front()) * 1e-6;
	const double avgHz = static_cast<double>(timestamps.size() - 1) / totalSeconds;
	const uint64_t minDt = dts.front();
	const uint64_t medianDt = dts[dts.size() / 2];
	const uint64_t p95Dt = dts[static_cast<std::size_t>(static_cast<double>(dts.size()) * 0.95)];
	const uint64_t maxDt = dts.back();

	std::println("samples: {}  duration: {:.2f}s  average: {:.1f} Hz", timestamps.size(), totalSeconds, avgHz);
	std::println("inter-sample gap (this is what matters for catching a brief event):");
	std::println("  min:    {:6} us  ({:6.1f} Hz)", minDt, 1e6 / static_cast<double>(minDt));
	std::println("  median: {:6} us  ({:6.1f} Hz)", medianDt, 1e6 / static_cast<double>(medianDt));
	std::println("  p95:    {:6} us  ({:6.1f} Hz)", p95Dt, 1e6 / static_cast<double>(p95Dt));
	std::println("  max:    {:6} us  ({:6.1f} Hz)  <- the floor", maxDt, 1e6 / static_cast<double>(maxDt));

	return 0;
}
