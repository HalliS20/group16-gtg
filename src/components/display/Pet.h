#ifndef PET_H
#define PET_H

#include <LEDMatrixDriver.h>

// smiley guy that goes sad when you don't go outside
class Pet {
public:
	static constexpr unsigned int width = 8;
	static constexpr unsigned int height = 8;

	enum class SadLevel : unsigned int { Happy = 0, Sad = 1, VerySad = 2 };

	static void draw(LEDMatrixDriver& driver, bool need_outside, bool need_movement);

private:
	static void drawBody(LEDMatrixDriver& driver);
	static void drawFace(LEDMatrixDriver& driver, SadLevel level);
	static void drawIndicators(LEDMatrixDriver& driver, bool need_outside, bool need_movement);
};

#endif
