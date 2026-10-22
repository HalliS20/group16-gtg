#ifndef DISPLAY_CONTROLLER_H
#define DISPLAY_CONTROLLER_H

#include <atomic>
#include <filesystem>
#include <optional>

#include <LEDMatrixDriver.h>

// Drives the LED matrix status display.
//
// Default: green/white checkerboard.
// insideTooLong:     green
// stationaryTooLong: yellow
// both:              red
class DisplayController {
public:
	bool init(const std::filesystem::path& fbDev, size_t fbSize, size_t bytesPerPixel,
		size_t width, size_t height, const std::filesystem::path& textAssetsPath,
		const std::filesystem::path& imageAssetsPath);

	void setInsideTooLong(bool tooLong);
	void setStationaryTooLong(bool tooLong);

private:
	std::optional<LEDMatrixDriver> m_driver;
	size_t m_width {0};
	size_t m_height {0};

	std::atomic<bool> m_insideTooLong {false};
	std::atomic<bool> m_stationaryTooLong {false};

	void updateDisplay();
};

#endif
