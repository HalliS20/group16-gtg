#include <iostream>
#include <unistd.h>

#include <LEDMatrixDriver.h>

#define SENSEHAT_FB_NAME "/dev/shm/rpi-sense-emu-screen"
#define SENSEHAT_FB_SIZE 160 // 160 for emulator, 128 for the real
#define SENSEHAT_FB_BYTES_PER_PX 2 // 16 bit per pixel RGB565
#define SENSEHAT_FB_WIDTH_PX 8
#define SENSEHAT_FB_HEIGH_PX 8

inline void run_ledmatrixdriver_example1()
{
	auto driverResult = LEDMatrixDriver::create(SENSEHAT_FB_NAME, SENSEHAT_FB_SIZE,
		SENSEHAT_FB_BYTES_PER_PX, SENSEHAT_FB_WIDTH_PX, SENSEHAT_FB_HEIGH_PX);
	if (!driverResult.has_value()) {
		std::cerr << driverResult.error() << '\n';
		return;
	}

	LEDMatrixDriver& driver = driverResult.value();

	driver.setPixel(0, 0, {.red = 255, .green = 0, .blue = 0});
	LEDMatrixDriver::LEDMatrixPixel px = driver.getPixel(0, 0);
	std::cout << "Pixel RGB: " << px.red << " " << px.green << " " << px.blue << std::endl;

	sleep(1);

	driver.setPixel(7, 7, {.red = 255, .green = 0, .blue = 0});
	px = driver.getPixel(7, 7);
	std::cout << "Pixel RGB: " << px.red << " " << px.green << " " << px.blue << std::endl;

	sleep(1);

	driver.setPixel(0, 0, {.red = 0, .green = 0, .blue = 0});
	driver.setPixel(7, 7, {.red = 0, .green = 0, .blue = 0});
}
