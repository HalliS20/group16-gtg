/**
 * @file LEDMatrixDriver.h
 * @brief
 * @version 0.1
 *
 *
 */

#ifndef _LEDMATRIXDRIVER_H
#define _LEDMATRIXDRIVER_H

#include <cstdint>
#include <expected>
#include <filesystem>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief
 *	Mirror(ish) of the official Python API -  https://github.com/astro-pi/python-sense-hat
 *	TODO: add more error handling with std::expected and proper error codes to avoid returning
 *  expensive std::array's.
 */
class LEDMatrixDriver {
public:
	/**
	 * @brief A simple struct to hold RGB colors.
	 *
	 */
	struct LEDMatrixRGBColor {
		unsigned int red, green, blue;
	};

	/**
	 * @brief Didn't like the idea of "storing a collection of RGB colors"
	 *
	 */
	using LEDMatrixPixel = LEDMatrixRGBColor;

	LEDMatrixDriver() = delete;
	LEDMatrixDriver(const LEDMatrixDriver&) = delete;
	LEDMatrixDriver& operator=(const LEDMatrixDriver&) = delete;

	LEDMatrixDriver(LEDMatrixDriver&& other) noexcept;
	LEDMatrixDriver& operator=(LEDMatrixDriver&& other) noexcept;

	~LEDMatrixDriver();

	/**
	 * @brief Initialize the driver. The reason a separate, returnable method is used, is to avoid
	 * performing non-trivial (possibly exceptional) operations in the driver's constructor.
	 *
	 * @param fbDev - path to the framebuffer.
	 * @param fbSize - size of the framebuffer in bytes.
	 * @param bytesPerPixel - bytes per pixel in the framebuffer.
	 * @param width - width of the screen represented by the framebuffer in pixels.
	 * @param height - height of the screen represented by the framebuffer in pixels.
	 * @param textAssetsPath - path to a .txt file containing a text string that corresponds to the imageAssets image.
	 * @param imageAssetsPath - path to an image containing letter pixels for text rendering.
	 * @return std::expected<LEDMatrixDriver, std::string> - an std::expected object, .has_value()
	 * is true if the call has succeeded (the actual driver object is then retrievable with
	 * .value()), otherwise check .error() for more details.
	 */
	static std::expected<LEDMatrixDriver, std::string> create(const std::filesystem::path& fbDev,
		size_t fbSize, size_t bytesPerPixel, size_t width, size_t height,
		const std::filesystem::path& textAssetsPath, const std::filesystem::path& imageAssetsPath);

	/**
	 * @brief Apply a counter-clockwise rotation. The rotation is applied on the current
	 * screen, and all subsequent screen updates.
	 * @note The rotation variable has to be divisible by 90. Applying variable rotations is
	 * problematic on smaller LEDs due to pixel aliasing.
	 * @param rotation - an integer divisible by 90.
	 */
	void rotateCCW(unsigned int rotation);

	/**
	 * @brief Get current rotation in degrees.
	 *
	 * @return int - current rotation.
	 */
	[[nodiscard]] unsigned int getRotation() const;

	/**
	 * @brief
	 *
	 * @param redraw
	 * @return true
	 * @return false
	 */
	std::vector<LEDMatrixPixel> flipH(bool redraw);

	/**
	 * @brief
	 *
	 * @param redraw
	 * @return true
	 * @return false
	 */
	std::vector<LEDMatrixPixel> flipV(bool redraw);

	/**
	 * @brief Set the LED pixels to the pixels supplied in the argument. If there are more supplied
	 * pixels than the current screen can support - the surplus pixels are cutoff.
	 *
	 * @param pixels - a collection of pixels.
	 */
	void setPixels(std::span<const LEDMatrixPixel> pixels);

	/**
	 * @brief Get pixels currently displayed (whenever possible) on the screen.
	 *
	 * @return std::vector<LEDMatrixPixel> - a collection of pixels.
	 */
	std::vector<LEDMatrixPixel> getPixels();

	/**
	 * @brief Set a single pixel at the coordinates (x,y) to a color specified by the color
	 * argument. This method applies the rotation that was set previously.
	 *
	 * @param x - x coordinate of a pixel.
	 * @param y - y coordinate of a pixel.
	 * @param color - the desired color of a pixel.
	 */
	void setPixel(unsigned int x, unsigned int y, const LEDMatrixRGBColor& color);

	/**
	 * @brief Get the current displayed (whenever possible) value of a pixel.
	 *
	 * @param x - x coordinate of a pixel.
	 * @param y - y coordinate of a pixel.
	 * @return LEDMatrixPixel - a struct describing a pixel.
	 */
	LEDMatrixPixel getPixel(unsigned int x, unsigned int y);

	/**
	 * @brief Renders a scrolling text on the LED screen.
	 *
	 * @param msg - text to render, only ASCII symbols.
	 * @param scrollSpeed - speed of the scrolling animation.
	 * @param textColor - color of the text.
	 * @param backColor - color of the background.
	 */
	void showMessage(const std::string& msg, float scrollSpeed, const LEDMatrixRGBColor& textColor,
		const LEDMatrixRGBColor& backColor);

	/**
	 * @brief Renders a single character on the LED screen.
	 *
	 * @param letter - character to render, only ASCII symbols.
	 * @param textColor - color of the character.
	 * @param backColor - color of the background.
	 */
	void showLetter(
		char letter, const LEDMatrixRGBColor& textColor, const LEDMatrixRGBColor& backColor);

	/**
	 * @brief Load an image specified by path, transform it into an array of LEDMatrixPixel objects.
	 * If redraw is set, renders the image on the screen.
	 * @note Use https://www.piskelapp.com/p/create/sprite/ to create 8x8 sprites.
	 * @param path - path to the image, files only.
	 * @param redraw - display the image on the LED screen.
	 * @return std::vector<LEDMatrixPixel> - an array of pixels representing the loaded image.
	 */
	std::vector<LEDMatrixPixel> loadImage(const std::filesystem::path& path, bool redraw);

	/**
	 * @brief Set the color of the screen. This resets rotation.
	 *
	 * @param color - the desired color of the screen.
	 */
	void clear(const LEDMatrixRGBColor& color);

private:
	LEDMatrixDriver(std::span<uint8_t> fbMmapAddr, int fbFd, size_t bytesPerPixel, size_t width,
		size_t height, std::unordered_map<char, std::vector<LEDMatrixPixel>> charsMap);

	std::span<uint8_t> m_fbMmapAddr;
	int m_fbFd;
	size_t m_bytesPerPixel;
	size_t m_width;
	size_t m_height;
	int m_rotation;
	std::unordered_map<char, std::vector<LEDMatrixPixel>> m_charsMap;
};

#endif
