#include "Pet.h"

static constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_BLACK {.red = 0, .green = 0, .blue = 0};
static constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_BODY {.red = 255, .green = 0, .blue = 255};
static constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_FACE {.red = 0, .green = 0, .blue = 0};
static constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_EYE {.red = 0, .green = 0, .blue = 0};
static constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_GREEN {.red = 0, .green = 255, .blue = 0};
static constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_BLUE {.red = 0, .green = 0, .blue = 255};
static constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_CYAN {.red = 0, .green = 255, .blue = 255};

// Body rows, top to bottom. 1 = body pixel.
static constexpr unsigned char bodyRows[Pet::height] = {
	0b00111100,
	0b01111110,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b01111110,
	0b00111100,
};

void Pet::draw(LEDMatrixDriver& driver, bool needOutside, bool needMovement) const
{
	driver.clear(COLOR_BLACK);

	const unsigned int sadLevel
		= (needOutside && needMovement) ? 2U : ((needOutside || needMovement) ? 1U : 0U);

	drawBody(driver, sadLevel);
	drawFace(driver, sadLevel);
	drawIndicators(driver, needOutside, needMovement);
}

void Pet::drawBody(LEDMatrixDriver& driver, unsigned int /*sadLevel*/) const
{
	for (unsigned int y = 0; y < Pet::height; ++y) {
		for (unsigned int x = 0; x < Pet::width; ++x) {
			if (bodyRows[y] & (1u << (Pet::width - 1 - x))) {
				driver.setPixel(x, y, COLOR_BODY);
			}
		}
	}
}

void Pet::drawFace(LEDMatrixDriver& driver, unsigned int sadLevel) const
{
	// eyes
	driver.setPixel(2, 2, COLOR_EYE);
	driver.setPixel(5, 2, COLOR_EYE);

	if (sadLevel == 0U) {
		// smile
		driver.setPixel(2, 4, COLOR_FACE);
		driver.setPixel(3, 5, COLOR_FACE);
		driver.setPixel(4, 5, COLOR_FACE);
		driver.setPixel(5, 4, COLOR_FACE);
	} else if (sadLevel == 1U) {
		// frown
		driver.setPixel(2, 5, COLOR_FACE);
		driver.setPixel(3, 4, COLOR_FACE);
		driver.setPixel(4, 4, COLOR_FACE);
		driver.setPixel(5, 5, COLOR_FACE);
	} else {
		// extra sad
		driver.setPixel(2, 5, COLOR_FACE);
		driver.setPixel(3, 4, COLOR_FACE);
		driver.setPixel(4, 4, COLOR_FACE);
		driver.setPixel(5, 5, COLOR_FACE);
		driver.setPixel(2, 1, COLOR_FACE);
		driver.setPixel(5, 1, COLOR_FACE);
		driver.setPixel(2, 3, COLOR_CYAN); // tear
	}
}

void Pet::drawIndicators(LEDMatrixDriver& driver, bool needOutside, bool needMovement) const
{
	if (needOutside) {
		driver.setPixel(0, Pet::height - 1, COLOR_GREEN); // bottom-left
	}
	if (needMovement) {
		driver.setPixel(Pet::width - 1, Pet::height - 1, COLOR_BLUE); // bottom-right
	}
}
