/**
 * @file Logger.hpp
 * @brief
 * @version 0.1
 *
 *
 */

#ifndef _LOGGER_HPP
#define _LOGGER_HPP

#include <iostream>
#include <string>

namespace Logger {

inline void logInfo(std::string module, std::string info)
{
	std::cout << "[INFO]\t" << module << ": " << info << std::endl;
}

inline void logError(std::string module, std::string error)
{
	std::cout << "[ERROR]\t" << module << ": " << error << std::endl;
}
inline void logDebug(std::string module, std::string debug)
{
	std::cout << "[DEBUG]\t" << module << ": " << debug << std::endl;
}
inline void logWarning(std::string module, std::string warning)
{
	std::cout << "[WARN]\t" << module << ": " << warning << std::endl;
}

} // namespace log
#endif
