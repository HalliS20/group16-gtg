#ifndef DISPLAY_CONTROLLER_H
#define DISPLAY_CONTROLLER_H

#include <optional>

#include <LEDMatrixDriver.h>

// Thin wrapper around the LED matrix driver exposing named status colors.
// Owns all sense-hat specific wiring (framebuffer device, asset paths).
class DisplayController {
public:
	bool init();

	void setWhite();
	void setGreen();
	void setBlue();
	void setRed();

private:
	std::optional<LEDMatrixDriver> m_driver;

	void setColor(const LEDMatrixDriver::LEDMatrixRGBColor& color);
};

#endif
