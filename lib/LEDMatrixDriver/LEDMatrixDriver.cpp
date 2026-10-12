/**
 * @file LEDMatrixDriver.cpp
 * @brief
 * @version 0.1
 *
 *
 */

#include <utility>
#include <iostream>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

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
		return std::unexpected("LEDMatrixDriver: Failed to truncate file descriptor to the requested length!\n");
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
			return std::unexpected("LEDMatrixDriver: Failed to close file descriptor of a fb device!\n");
		}
	}
	return 0;
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
