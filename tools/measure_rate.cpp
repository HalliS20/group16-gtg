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
#include <cstdint>
#include <cstdlib>
#include <print>
#include <unistd.h>
#include <vector>

#include "RTIMULib.h"

namespace {

RTIMU* initIMU()
{
	RTIMUSettings* settings = new RTIMUSettings("RTIMULib");
	RTIMU* imu = RTIMU::createIMU(settings);
	if ((imu == nullptr) || (imu->IMUType() == RTIMU_TYPE_NULL)) {
		std::println("No IMU found");
		exit(1);
	}

	imu->IMUInit();
	imu->setSlerpPower(0.02);
	imu->setGyroEnable(true);
	imu->setAccelEnable(true);
	imu->setCompassEnable(true);
	return imu;
}

} // namespace

int main(int argc, char** argv)
{
	const double runSeconds = argc > 1 ? std::atof(argv[1]) : 15.0;

	RTIMU* imu = initIMU();
	std::println("configured poll interval: {} ms", imu->IMUGetPollInterval());

	std::vector<uint64_t> timestamps;
	timestamps.reserve(20000);

	const auto start = std::chrono::steady_clock::now();
	while (std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count() < runSeconds) {
		usleep(imu->IMUGetPollInterval() * 1000);
		while (imu->IMURead()) {
			const RTIMU_DATA& imuData = imu->getIMUData();
			if (!imuData.accelValid) {
				continue;
			}
			timestamps.push_back(imuData.timestamp);
		}
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
