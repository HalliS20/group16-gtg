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

/**
 * @brief
 *	Mirror(ish) of the official Python API -  https://github.com/astro-pi/python-sense-hat
 */
class LEDMatrixDriver {
public:
	LEDMatrixDriver() = delete;
	LEDMatrixDriver(const LEDMatrixDriver&) = delete;
	LEDMatrixDriver& operator=(const LEDMatrixDriver&) = delete;

	LEDMatrixDriver(LEDMatrixDriver&& other) noexcept;
	LEDMatrixDriver& operator=(LEDMatrixDriver&& other) noexcept;

	~LEDMatrixDriver();

	/**
	 * @brief
	 *
	 * @param fbDev
	 * @param fbSize
	 * @param bytesPerPixel
	 * @param width
	 * @param height
	 * @return std::expected<LEDMatrixDriver, std::string>
	 */
	static std::expected<LEDMatrixDriver, std::string> create(const std::filesystem::path& fbDev,
		size_t fbSize, size_t bytesPerPixel, size_t width, size_t height);

private:
	LEDMatrixDriver(
		std::span<uint8_t> fbMmapAddr, int fbFd, size_t bytesPerPixel, size_t width, size_t height);

	std::span<uint8_t> m_fbMmapAddr;
	int m_fbFd;
	size_t m_bytesPerPixel;
	size_t m_width;
	size_t m_height;
	unsigned int m_rotation;
};

#endif
