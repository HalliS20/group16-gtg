#include "Pet.h"

#include <array>
#include <bitset>

static constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_BLACK {.red = 0, .green = 0, .blue = 0};
static constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_BODY {.red = 255, .green = 165, .blue = 0};
static constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_FACE {.red = 0, .green = 0, .blue = 0};
static constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_EYE {.red = 0, .green = 0, .blue = 0};
static constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_GREEN {.red = 0, .green = 255, .blue = 0};
static constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_BLUE {.red = 0, .green = 0, .blue = 255};
static constexpr LEDMatrixDriver::LEDMatrixRGBColor COLOR_CYAN {.red = 0, .green = 255, .blue = 255};

// Body rows, top to bottom. 1 = body pixel.
static constexpr std::array<unsigned char, Pet::height> body_rows {
	0b00111100,
	0b01111110,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b01111110,
	0b00111100,
};

void Pet::draw(LEDMatrixDriver& driver, bool need_outside, bool need_movement)
{
	driver.clear(COLOR_BLACK);

	SadLevel sad_level = SadLevel::Happy;
	if (need_outside && need_movement) {
		sad_level = SadLevel::VerySad;
	} else if (need_outside || need_movement) {
		sad_level = SadLevel::Sad;
	}

	drawBody(driver);
	drawFace(driver, sad_level);
	drawIndicators(driver, need_outside, need_movement);
}

void Pet::drawBody(LEDMatrixDriver& driver)
{
	for (unsigned int y = 0; y < Pet::height; ++y) {
		const std::bitset<Pet::width> row(body_rows[y]);
		for (unsigned int x = 0; x < Pet::width; ++x) {
			if (row.test(Pet::width - 1 - x)) {
				driver.setPixel(x, y, COLOR_BODY);
			}
		}
	}
}

void Pet::drawFace(LEDMatrixDriver& driver, SadLevel level)
{
	// eyes
	driver.setPixel(2, 2, COLOR_EYE);
	driver.setPixel(5, 2, COLOR_EYE);

	switch (level) {
	case SadLevel::Happy:
		// smile
		driver.setPixel(2, 4, COLOR_FACE);
		driver.setPixel(3, 5, COLOR_FACE);
		driver.setPixel(4, 5, COLOR_FACE);
		driver.setPixel(5, 4, COLOR_FACE);
		break;
	case SadLevel::Sad:
		// flat mouth
		driver.setPixel(2, 5, COLOR_FACE);
		driver.setPixel(3, 5, COLOR_FACE);
		driver.setPixel(4, 5, COLOR_FACE);
		driver.setPixel(5, 5, COLOR_FACE);
		break;
	case SadLevel::VerySad:
		// frown, brows, tear
		driver.setPixel(2, 5, COLOR_FACE);
		driver.setPixel(3, 4, COLOR_FACE);
		driver.setPixel(4, 4, COLOR_FACE);
		driver.setPixel(5, 5, COLOR_FACE);
		driver.setPixel(2, 1, COLOR_FACE);
		driver.setPixel(5, 1, COLOR_FACE);
		driver.setPixel(6, 3, COLOR_CYAN);
		break;
	}
}

void Pet::drawIndicators(LEDMatrixDriver& driver, bool need_outside, bool need_movement)
{
	if (need_outside) {
		driver.setPixel(0, Pet::height - 1, COLOR_GREEN); // bottom-left
		driver.setPixel(1, Pet::height - 1, COLOR_GREEN);
		driver.setPixel(0, Pet::height - 2, COLOR_GREEN);
	}
	if (need_movement) {
		driver.setPixel(Pet::width - 1, Pet::height - 1, COLOR_BLUE); // bottom-right
		driver.setPixel(Pet::width - 2, Pet::height - 1, COLOR_BLUE);
		driver.setPixel(Pet::width - 1, Pet::height - 2, COLOR_BLUE);
	}
}
