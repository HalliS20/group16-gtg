#include "DisplayController.h"

#include <utility>

static constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_GREEN {.red = 0, .green = 255, .blue = 0};
static constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_OFF {.red = 0, .green = 0, .blue = 0};
static constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_WHITE {
	.red = 255, .green = 255, .blue = 255};
static constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_BLUE {.red = 0, .green = 0, .blue = 255};
static constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_RED {.red = 255, .green = 0, .blue = 0};

// for the emulator, use "/dev/shm/rpi-sense-emu-screen" with size 160 instead.
// check /sys/class/graphics/fb*/name to find the correct fb on real hardware.
static constexpr const char* FB_NAME = "/dev/fb1";
static constexpr size_t FB_SIZE = 128LU; // 160 for emulator, 128 for the real
static constexpr size_t FB_BYTES_PER_PX = 2LU; // 16 bit per pixel RGB565
static constexpr size_t FB_WIDTH_PX = 8LU;
static constexpr size_t FB_HEIGHT_PX = 8LU;
static constexpr const char* TEXT_FILE_ASSET_PATH = "assets/sense_hat_text.txt";
static constexpr const char* IMAGE_FILE_ASSET_PATH = "assets/sense_hat_text.png";

void DisplayController::setColor(const LEDMatrixDriver::LEDMatrixRGBColor& color)
{
	if (!m_driver.has_value()) {
		return;
	}
	m_driver->clear(color);
}

void DisplayController::setRed()
{
	setColor(COLOR_RED);
}

void DisplayController::setGreen()
{
	setColor(COLOR_GREEN);
}

void DisplayController::setBlue()
{
	setColor(COLOR_BLUE);
}

void DisplayController::setOff()
{
	setColor(COLOR_OFF);
}

void DisplayController::setWhite()
{
	setColor(COLOR_WHITE);
}

bool DisplayController::init()
{
	std::expected<LEDMatrixDriver, std::string> driverResult
		= LEDMatrixDriver::create(FB_NAME, FB_SIZE, FB_BYTES_PER_PX, FB_WIDTH_PX, FB_HEIGHT_PX,
			TEXT_FILE_ASSET_PATH, IMAGE_FILE_ASSET_PATH);
	if (!driverResult.has_value()) {
		return false;
	}

	m_driver.emplace(std::move(driverResult.value()));
	setOff();
	return true;
}
