#include "DisplayController.h"

#include "Pet.h"

// for the emulator, use "/dev/shm/rpi-sense-emu-screen" with size 160 instead.
// check /sys/class/graphics/fb*/name to find the correct fb on real hardware.
static constexpr const char* FB_NAME = "/dev/fb1";
static constexpr size_t FB_SIZE = 128LU; // 160 for emulator, 128 for the real
static constexpr size_t FB_BYTES_PER_PX = 2LU; // 16 bit per pixel RGB565
static constexpr size_t FB_WIDTH_PX = 8LU;
static constexpr size_t FB_HEIGHT_PX = 8LU;
static constexpr const char* TEXT_FILE_ASSET_PATH = "assets/sense_hat_text.txt";
static constexpr const char* IMAGE_FILE_ASSET_PATH = "assets/sense_hat_text.png";

LEDMatrixDriver DisplayController::createDriver()
{
	return LEDMatrixDriver::create(FB_NAME, FB_SIZE, FB_BYTES_PER_PX, FB_WIDTH_PX, FB_HEIGHT_PX,
		TEXT_FILE_ASSET_PATH, IMAGE_FILE_ASSET_PATH)
		.value();
}

void DisplayController::showPet(bool need_outside, bool need_movement)
{
	Pet::draw(m_driver, need_outside, need_movement);
}

DisplayController::DisplayController()
{
	showPet(false, false);
}
