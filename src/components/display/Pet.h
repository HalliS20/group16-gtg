#ifndef PET_H
#define PET_H

#include <LEDMatrixDriver.h>

// 8x8 pixel companion with status indicators:
// - happy face by default
// - sad face when either indicator is active
// - green bottom-left: go outside
// - blue bottom-right: move around
class Pet {
public:
	static constexpr unsigned int width = 8;
	static constexpr unsigned int height = 8;

	void draw(LEDMatrixDriver& driver, bool needOutside, bool needMovement) const;

private:
	void drawBody(LEDMatrixDriver& driver, unsigned int sadLevel) const;
	void drawFace(LEDMatrixDriver& driver, unsigned int sadLevel) const;
	void drawIndicators(LEDMatrixDriver& driver, bool needOutside, bool needMovement) const;
};

#endif
