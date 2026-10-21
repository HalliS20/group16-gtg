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

inline void logInfo(const std::string& module, const std::string& info)
{
	std::cout << "[INFO]\t" << module << ": " << info << std::endl;
}

inline void logError(const std::string& module, const std::string& error)
{
	std::cout << "[ERROR]\t" << module << ": " << error << std::endl;
}
inline void logDebug(const std::string& module, const std::string& debug)
{
	std::cout << "[DEBUG]\t" << module << ": " << debug << std::endl;
}
inline void logWarning(const std::string& module, const std::string& warning)
{
	std::cout << "[WARN]\t" << module << ": " << warning << std::endl;
}

} // namespace Logger
#endif
