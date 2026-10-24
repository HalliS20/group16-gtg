#ifndef DISPLAY_CONTROLLER_H
#define DISPLAY_CONTROLLER_H

#include <optional>

#include <LEDMatrixDriver.h>

// Owns all sense-hat specific wiring (framebuffer device, asset paths).
class DisplayController {
public:
	bool init();

	void setOff();
	void setWhite();
	void setGreen();
	void setBlue();
	void setRed();

private:
	std::optional<LEDMatrixDriver> m_driver;

	void setColor(const LEDMatrixDriver::LEDMatrixRGBColor& color);
};

#endif
