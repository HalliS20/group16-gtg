/**
 * @file LEDMatrixDriver.cpp
 * @brief
 * @version 0.1
 *
 *
 */

#include <cmath>
#include <iostream>
#include <utility>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "LEDMatrixDriver.h"

namespace fs = std::filesystem;

namespace {
bool checkFBDevice(const std::string& fbDev)
{
	return fs::exists(fbDev);
}

std::expected<int, std::string> openFBDevice(const fs::path& fbDevPath, size_t size)
{
	int fbFd = open(fbDevPath.c_str(), O_RDWR);
	if (fbFd < 0) {
		return std::unexpected("LEDMatrixDriver: Failed to open file descriptor of a fb device!\n");
	}
	if (0 != ftruncate(fbFd, size)) {
		return std::unexpected(
			"LEDMatrixDriver: Failed to truncate file descriptor to the requested length!\n");
	}
	return fbFd;
}

std::expected<std::span<uint8_t>, std::string> mmapFBDevice(int fbFd, size_t size)
{
	void* mem = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fbFd, 0);
	if (MAP_FAILED == mem) {
		return std::unexpected("LEDMatrixDriver: Failed to mmap a fb device!\n");
	}
	return std::span<uint8_t>(static_cast<uint8_t*>(mem), size);
}

std::expected<int, std::string> unmapFBDevice(std::span<uint8_t> mmapedDevice)
{
	if (!mmapedDevice.empty()) {
		if (0 != munmap(mmapedDevice.data(), mmapedDevice.size_bytes())) {
			return std::unexpected("LEDMatrixDriver: Failed to unmmap a fb device!\n");
		}
	}
	return 0;
}
std::expected<int, std::string> closeFBDevice(int fbFd)
{
	if (fbFd > 0) {
		if (0 != close(fbFd)) {
			return std::unexpected(
				"LEDMatrixDriver: Failed to close file descriptor of a fb device!\n");
		}
	}
	return 0;
}

uint32_t rescaleRange(uint32_t val, uint32_t max_in, uint32_t max_out)
{
	return ((val * max_out) + (max_in / 2)) / max_in;
}

LEDMatrixDriver::LEDMatrixRGBColor rgb888ToRgb565(uint8_t red, uint8_t green, uint8_t blue)
{
	uint32_t r5 = rescaleRange(red, 255, 31); // 2^8, 2^5
	uint32_t g6 = rescaleRange(green, 255, 63); // 2^8, 2^6
	uint32_t b5 = rescaleRange(blue, 255, 31); // 2^8, 2^5

	return {.red = r5, .green = g6, .blue = b5};
}

uint16_t packRgb565(uint8_t red, uint8_t green, uint8_t blue)
{
	return ((red << 11) & 0xF800) | ((green << 5) & 0x07E0) | (blue & 0x001F);
}

LEDMatrixDriver::LEDMatrixRGBColor unpackRgb565(uint16_t packedRgb)
{
	uint8_t r5 = (packedRgb & 0xF800) >> 11;
	uint8_t g6 = (packedRgb & 0x7E0) >> 5;
	uint8_t b5 = (packedRgb & 0x1F);

	return {.red = r5, .green = g6, .blue = b5};
}

LEDMatrixDriver::LEDMatrixRGBColor rgb565ToRgb888(uint8_t red, uint8_t green, uint8_t blue)
{
	uint32_t r8 = rescaleRange(red, 31, 255); // 2^8, 2^5
	uint32_t g8 = rescaleRange(green, 63, 255); // 2^8, 2^6
	uint32_t b8 = rescaleRange(blue, 31, 255); // 2^8, 2^5

	return {.red = r8, .green = g8, .blue = b8};
}

unsigned int rotatePixel90degCCW(
	unsigned int i, size_t width, size_t height, unsigned int numRotations)
{
	unsigned int oldI = i;
	unsigned int newI = oldI;
	while (0 != (numRotations--)) {
		unsigned int xOld = oldI % width;
		unsigned int yOld = std::floor(oldI / height);

		unsigned int xNew = yOld;
		unsigned int yNew = (width - 1) - xOld;

		newI = (yNew * width) + xNew;
		oldI = newI;
	}

	return newI;
}
} // namespace

std::expected<LEDMatrixDriver, std::string> LEDMatrixDriver::create(
	const fs::path& fbDev, size_t fbSize, size_t bytesPerPixel, size_t width, size_t height)
{
	bool validFb = checkFBDevice(fbDev);
	if (!validFb) {
		return std::unexpected("LEDMatrixDriver: Invalid fb device!\n");
	}

	std::cout << "LEDMatrixDriver: Using " << fbDev << " device.\n";

	auto fbFd = openFBDevice(fbDev, fbSize);
	if (!fbFd.has_value()) {
		return std::unexpected(fbFd.error());
	}

	std::cout << "LEDMatrixDriver: Opened " << fbDev << " device for read/write.\n";

	auto fbMmapPtr = mmapFBDevice(fbFd.value(), fbSize);
	if (!fbMmapPtr.has_value()) {
		closeFBDevice(fbFd.value());
		return std::unexpected(fbMmapPtr.error());
	}

	std::cout << "LEDMatrixDriver: Mmaped " << fbDev << " device.\n";

	return LEDMatrixDriver(fbMmapPtr.value(), fbFd.value(), bytesPerPixel, width, height);
}
LEDMatrixDriver::LEDMatrixDriver(
	std::span<uint8_t> fbMmapAddr, int fbFd, size_t bytesPerPixel, size_t width, size_t height)
	: m_fbMmapAddr(fbMmapAddr)
	, m_fbFd(fbFd)
	, m_bytesPerPixel(bytesPerPixel)
	, m_width(width)
	, m_height(height)
	, m_rotation(0)
{
}
LEDMatrixDriver::LEDMatrixDriver(LEDMatrixDriver&& other) noexcept
	: m_fbMmapAddr(std::exchange(other.m_fbMmapAddr, {}))
	, m_fbFd(std::exchange(other.m_fbFd, -1))
	, m_bytesPerPixel(other.m_bytesPerPixel)
	, m_width(other.m_width)
	, m_height(other.m_height)
	, m_rotation(other.m_rotation)
{
}
LEDMatrixDriver& LEDMatrixDriver::operator=(LEDMatrixDriver&& other) noexcept
{
	if (this != &other) {
		unmapFBDevice(m_fbMmapAddr);
		closeFBDevice(m_fbFd);

		m_fbMmapAddr = std::exchange(other.m_fbMmapAddr, {});
		m_fbFd = std::exchange(other.m_fbFd, -1);
		m_bytesPerPixel = other.m_bytesPerPixel;
		m_rotation = other.m_rotation;
		m_width = other.m_width;
		m_height = other.m_height;
	}
	return *this;
}
LEDMatrixDriver::~LEDMatrixDriver()
{
	unmapFBDevice(m_fbMmapAddr);
	closeFBDevice(m_fbFd);
}

void LEDMatrixDriver::setPixel(unsigned int x, unsigned int y, const LEDMatrixRGBColor& color)
{
	if (x > m_width - 1 || y > m_height - 1) {
		std::cerr << "LEDMatrixDriver: Invalid pixel coordinates!" << std::endl;
		return;
	}
	if (m_fbMmapAddr.empty()) {
		std::cerr << "LEDMatrixDriver: Fb memory address is null!" << std::endl;
		return;
	}

	unsigned int newI = rotatePixel90degCCW((m_width * y) + x, m_width, m_height, m_rotation / 90);

	size_t offset = newI * m_bytesPerPixel;

	LEDMatrixRGBColor color565 = rgb888ToRgb565(color.red, color.green, color.blue);
	uint16_t value = packRgb565(color565.red, color565.green, color565.blue);

	for (size_t byteOffset = 0; byteOffset < m_bytesPerPixel; ++byteOffset) {
		if (offset + byteOffset > m_fbMmapAddr.size_bytes()) {
			std::cerr << "LEDMatrixDriver: offset+byteOffset outside of the fb memory range!"
					  << std::endl;
			break;
		}
		m_fbMmapAddr[offset + byteOffset] = static_cast<uint8_t>((value >> (8 * byteOffset)) & 0xFF);
	}
}

LEDMatrixDriver::LEDMatrixPixel LEDMatrixDriver::getPixel(unsigned int x, unsigned int y)
{
	if (x > m_width - 1 || y > m_height - 1) {
		std::cerr << "LEDMatrixDriver: Invalid pixel coordinates!" << std::endl;
		return {};
	}
	if (m_fbMmapAddr.empty()) {
		std::cerr << "LEDMatrixDriver: Fb memory address is null!" << std::endl;
		return {};
	}

	size_t offset = (y * m_bytesPerPixel * m_width) + (x * m_bytesPerPixel);

	uint16_t value = 0;

	for (size_t byteOffset = 0; byteOffset < m_bytesPerPixel; ++byteOffset) {
		if (offset + byteOffset > m_fbMmapAddr.size_bytes()) {
			std::cerr << "LEDMatrixDriver: offset+byteOffset outside of the fb memory range!"
					  << std::endl;
			break;
		}

		value |= (m_fbMmapAddr[offset + byteOffset] << (8 * byteOffset));
	}

	LEDMatrixRGBColor unpackedValue = unpackRgb565(value);

	return rgb565ToRgb888(unpackedValue.red, unpackedValue.green, unpackedValue.blue);
}

void LEDMatrixDriver::setPixels(std::span<const LEDMatrixPixel> pixels)
{
	for (size_t i = 0; i < m_width * m_height; ++i) {
		if (i >= pixels.size()) {
			break;
		}
		setPixel(i % m_width, std::floor(i / m_height), pixels[i]);
	}
}

std::vector<LEDMatrixDriver::LEDMatrixPixel> LEDMatrixDriver::getPixels()
{
	std::vector<LEDMatrixPixel> pixels(m_width * m_height, {.red = 0, .green = 0, .blue = 0});

	for (size_t i = 0; i < m_width * m_height; ++i) {
		if (i >= pixels.size()) {
			break;
		}
		pixels.at(i) = getPixel(i % m_width, std::floor(i / m_height));
	}
	return pixels;
}

void LEDMatrixDriver::clear(const LEDMatrixRGBColor& color)
{
	m_rotation = 0;

	std::vector<LEDMatrixPixel> pixels(m_width * m_height, color);
	setPixels(pixels);
}

void LEDMatrixDriver::rotateCCW(unsigned int rotation)
{
	if (rotation % 90 == 0) {
		std::vector<LEDMatrixPixel> pixels = getPixels();

		m_rotation = rotation;

		setPixels(pixels);
	}
}

unsigned int LEDMatrixDriver::getRotation() const
{
	return m_rotation;
}

std::vector<LEDMatrixDriver::LEDMatrixPixel> LEDMatrixDriver::flipH(bool redraw)
{
	std::vector<LEDMatrixDriver::LEDMatrixPixel> pixelList = getPixels();
	std::vector<LEDMatrixDriver::LEDMatrixPixel> flipped;

	for (int i = 0; i < (int)m_height; ++i) {
		for (int j = ((int)m_width - 1); j >= 0; --j) {
			flipped.push_back(pixelList.at((i * (int)m_width) + j));
		}
	}

	if (redraw) {
		setPixels(flipped);
	}

	return flipped;
}

std::vector<LEDMatrixDriver::LEDMatrixPixel> LEDMatrixDriver::flipV(bool redraw)
{
	std::vector<LEDMatrixDriver::LEDMatrixPixel> pixelList = getPixels();
	std::vector<LEDMatrixDriver::LEDMatrixPixel> flipped;

	for (int i = ((int)m_height - 1); i >= 0; --i) {
		for (int j = 0; j < (int)m_width; ++j) {
			flipped.push_back(pixelList.at((i * (int)m_width) + j));
		}
	}

	if (redraw) {
		setPixels(flipped);
	}

	return flipped;
}

std::vector<LEDMatrixDriver::LEDMatrixPixel> LEDMatrixDriver::loadImage(
	const fs::path& path, bool redraw)
{
	if (!fs::exists(path) || !fs::is_regular_file(path)) {
		std::cerr << "LEDMatrixDriver: Invalid file path!" << std::endl;
		return {};
	}

	int width = 0;
	int height = 0;
	int channels = 0;
	int desired_channels = 3;
	auto* imageDataRaw = stbi_load(path.c_str(), &width, &height, &channels, desired_channels);
	if (nullptr == imageDataRaw) {
		std::cerr << "LEDMatrixDriver: Failed to load image - " << stbi_failure_reason() << "!" << std::endl;
		return {};
	}

	size_t numPixels = width * height;
	size_t numBytes = numPixels * desired_channels;

	auto imageData = std::span<unsigned char>(static_cast<unsigned char*>(imageDataRaw), numBytes);

	std::vector<LEDMatrixPixel> pixels(numPixels, {.red = 0, .green = 0, .blue = 0});

	for (size_t i = 0; i < numBytes; i += desired_channels) {
		uint8_t r8 = imageData[i];
		uint8_t g8 = imageData[i + 1];
		uint8_t b8 = imageData[i + 2];

		LEDMatrixPixel pixel = {.red = r8, .green = g8, .blue = b8};
		pixels.at(i / desired_channels) = pixel;
	}

	if (redraw) {
		setPixels(pixels);
	}

	stbi_image_free(imageDataRaw);

	return pixels;
}
