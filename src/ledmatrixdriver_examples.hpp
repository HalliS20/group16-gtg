#include <iostream>
#include <unistd.h>

#include <LEDMatrixDriver.h>

// different for the real sense hat, check /sys/class/graphics/fb*/name to find the correct fb, then use "/dev/fbx".
#define SENSEHAT_FB_NAME "/dev/shm/rpi-sense-emu-screen"
#define SENSEHAT_FB_SIZE 160LU // 160 for emulator, 128 for the real
#define SENSEHAT_FB_BYTES_PER_PX 2LU // 16 bit per pixel RGB565
#define SENSEHAT_FB_WIDTH_PX 8LU
#define SENSEHAT_FB_HEIGHT_PX 8LU

static const std::array<LEDMatrixDriver::LEDMatrixPixel, SENSEHAT_FB_HEIGHT_PX * SENSEHAT_FB_WIDTH_PX> demoPixels2
	= {{
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 0, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 0, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 0, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 0, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
		{.red = 255, .green = 0, .blue = 0},
	}};

inline void run_ledmatrixdriver_example1()
{
	auto driverResult = LEDMatrixDriver::create(SENSEHAT_FB_NAME, SENSEHAT_FB_SIZE,
		SENSEHAT_FB_BYTES_PER_PX, SENSEHAT_FB_WIDTH_PX, SENSEHAT_FB_HEIGHT_PX);
	if (!driverResult.has_value()) {
		std::cerr << driverResult.error() << '\n';
		return;
	}

	LEDMatrixDriver& driver = driverResult.value();

	driver.setPixel(0, 0, {.red = 255, .green = 0, .blue = 0});
	LEDMatrixDriver::LEDMatrixPixel crntPixel = driver.getPixel(0, 0);
	std::cout << "Pixel RGB: " << crntPixel.red << " " << crntPixel.green << " " << crntPixel.blue
			  << std::endl;

	sleep(1);

	driver.setPixel(7, 7, {.red = 255, .green = 0, .blue = 0});
	crntPixel = driver.getPixel(7, 7);
	std::cout << "Pixel RGB: " << crntPixel.red << " " << crntPixel.green << " " << crntPixel.blue
			  << std::endl;

	sleep(1);

	driver.setPixel(0, 0, {.red = 0, .green = 0, .blue = 0});
	driver.setPixel(7, 7, {.red = 0, .green = 0, .blue = 0});

	sleep(1);

	std::vector<LEDMatrixDriver::LEDMatrixPixel> demoPixels1(
		SENSEHAT_FB_WIDTH_PX * SENSEHAT_FB_HEIGHT_PX, {.red = 255, .green = 0, .blue = 255});
	driver.setPixels(demoPixels1);

	std::vector<LEDMatrixDriver::LEDMatrixPixel> crntPixels = driver.getPixels();

	for (long unsigned i = 0U; i < SENSEHAT_FB_HEIGHT_PX; ++i) {
		for (long unsigned j = 0U; j < SENSEHAT_FB_WIDTH_PX; ++j) {
			crntPixel = crntPixels.at((SENSEHAT_FB_WIDTH_PX * i) + j);
			std::cout << "(" << crntPixel.red << "," << crntPixel.green << "," << crntPixel.blue
					  << ") ";
		}
		std::cout << std::endl;
	}

	sleep(1);

	driver.clear({.red = 0, .green = 0, .blue = 0});

	sleep(1);

	driver.setPixels(demoPixels2);
	sleep(1);
	driver.rotateCCW(90);
	sleep(1);
	driver.rotateCCW(90);
	sleep(1);
	driver.rotateCCW(90);
	sleep(1);
	driver.rotateCCW(90);
	sleep(1);

	driver.clear({.red = 0, .green = 0, .blue = 0});
}
