#ifndef DISPLAY_CONTROLLER_H
#define DISPLAY_CONTROLLER_H

#include <LEDMatrixDriver.h>

// Owns all sense-hat specific wiring (framebuffer device, asset paths).
class DisplayController {
public:
	DisplayController();

	void showPet(bool need_outside, bool need_movement);

private:
	static LEDMatrixDriver createDriver();

	LEDMatrixDriver m_driver = createDriver();
};

#endif
