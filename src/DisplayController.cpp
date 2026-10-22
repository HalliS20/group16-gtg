#include "DisplayController.h"

#include <utility>
#include <vector>

namespace {

constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_GREEN {.red = 0, .green = 255, .blue = 0};
constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_WHITE {.red = 255, .green = 255, .blue = 255};
constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_YELLOW {.red = 255, .green = 255, .blue = 0};
constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_RED {.red = 255, .green = 0, .blue = 0};

} // namespace

bool DisplayController::init(const std::filesystem::path& fbDev, size_t fbSize,
	size_t bytesPerPixel, size_t width, size_t height, const std::filesystem::path& textAssetsPath,
	const std::filesystem::path& imageAssetsPath)
{
	auto driverResult
		= LEDMatrixDriver::create(fbDev, fbSize, bytesPerPixel, width, height, textAssetsPath, imageAssetsPath);
	if (!driverResult.has_value()) {
		return false;
	}

	m_driver.emplace(std::move(driverResult.value()));
	m_width = width;
	m_height = height;
	updateDisplay();
	return true;
}

void DisplayController::setInsideTooLong(bool tooLong)
{
	if (m_insideTooLong.exchange(tooLong) != tooLong) {
		updateDisplay();
	}
}

void DisplayController::setStationaryTooLong(bool tooLong)
{
	if (m_stationaryTooLong.exchange(tooLong) != tooLong) {
		updateDisplay();
	}
}

void DisplayController::updateDisplay()
{
	if (!m_driver.has_value()) {
		return;
	}

	const bool insideTooLong = m_insideTooLong.load();
	const bool stationaryTooLong = m_stationaryTooLong.load();

	if (insideTooLong && stationaryTooLong) {
		m_driver->clear(COLOR_RED);
	} else if (insideTooLong) {
		m_driver->clear(COLOR_GREEN);
	} else if (stationaryTooLong) {
		m_driver->clear(COLOR_YELLOW);
	} else {
		std::vector<LEDMatrixDriver::LEDMatrixPixel> checkerboard;
		checkerboard.reserve(m_width * m_height);
		for (size_t y = 0; y < m_height; ++y) {
			for (size_t x = 0; x < m_width; ++x) {
				checkerboard.push_back(((x + y) % 2 == 0) ? COLOR_GREEN : COLOR_WHITE);
			}
		}
		m_driver->setPixels(checkerboard);
	}
}
