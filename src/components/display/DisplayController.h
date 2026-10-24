#ifndef DISPLAY_CONTROLLER_H
#define DISPLAY_CONTROLLER_H

#include <optional>

#include <LEDMatrixDriver.h>

#include "Pet.h"

// Owns all sense-hat specific wiring (framebuffer device, asset paths).
class DisplayController {
public:
	bool init();

	void setWhite();
	void setGreen();
	void setBlue();
	void setRed();

	// Draw the pet with status indicators.
	void showPet(bool needOutside, bool needMovement);

private:
	std::optional<LEDMatrixDriver> m_driver;
	Pet m_pet;

	void setColor(const LEDMatrixDriver::LEDMatrixRGBColor& color);
};

#endif
