#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <string>

namespace Logger {

void logInfo(const std::string& module, const std::string& info);
void logError(const std::string& module, const std::string& error);
void logDebug(const std::string& module, const std::string& debug);
void logWarning(const std::string& module, const std::string& warning);

} // namespace Logger

inline void Logger::logInfo(const std::string& module, const std::string& info)
{
	std::cout << "[INFO]\t" << module << ": " << info << std::endl;
}

inline void Logger::logError(const std::string& module, const std::string& error)
{
	std::cout << "[ERROR]\t" << module << ": " << error << std::endl;
}

inline void Logger::logDebug(const std::string& module, const std::string& debug)
{
	std::cout << "[DEBUG]\t" << module << ": " << debug << std::endl;
}

inline void Logger::logWarning(const std::string& module, const std::string& warning)
{
	std::cout << "[WARN]\t" << module << ": " << warning << std::endl;
}

#endif
