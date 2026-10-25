#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

namespace Logger {

void logInfo(const string& module, const string& info);
void logError(const string& module, const string& error);
void logDebug(const string& module, const string& debug);
void logWarning(const string& module, const string& warning);

} // namespace Logger

inline void Logger::logInfo(const std::string& module, const std::string& info)
{
	cout << "[INFO]\t" << module << ": " << info << endl;
}

inline void Logger::logError(const std::string& module, const std::string& error)
{
	cout << "[ERROR]\t" << module << ": " << error << endl;
}

inline void Logger::logDebug(const std::string& module, const std::string& debug)
{
	cout << "[DEBUG]\t" << module << ": " << debug << endl;
}

inline void Logger::logWarning(const std::string& module, const std::string& warning)
{
	cout << "[WARN]\t" << module << ": " << warning << endl;
}

#endif
