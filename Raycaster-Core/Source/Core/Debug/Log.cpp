#include "Log.h"

#include <chrono>
#include <thread>

constexpr auto GRAY = "\033[90m";
constexpr auto RESET = "\033[0m";

namespace Core {
	constexpr const char* Logger::GetLogLevelString(LogLevel logLevel) {
		switch (logLevel) {
		case LogLevel::Trace:
			return "TRACE";
		case LogLevel::Info:
			return " INFO";
		case LogLevel::Warn:
			return " WARN";
		case LogLevel::Error:
			return "ERROR";
		case LogLevel::Fatal:
			return "FATAL";
		}
		return "";
	}

	constexpr const char* Logger::GetLogLevelColour(LogLevel logLevel) {
		switch (logLevel) {
		case LogLevel::Trace:
			return "\033[36m";
		case LogLevel::Info:
			return "\033[32m";
		case LogLevel::Warn:
			return "\033[33m";
		case LogLevel::Error:
			return "\033[31m";
		case LogLevel::Fatal:
			return "\033[97;41m";
		}
		return "";
	}

	std::string Logger::CreatePrefix(LogLevel logLevel, PrefixLevel prefixLevel, const std::source_location& sourceLocation) {
		std::stringstream result;

		if (prefixLevel >= PrefixLevel::Reduced) {
			result << std::format("{:%T} ", floor<std::chrono::seconds>(std::chrono::system_clock::now()));
		}

		result << GetLogLevelColour(logLevel) << GetLogLevelString(logLevel) << RESET << " ";
		
		if (prefixLevel >= PrefixLevel::Full) {
			std::filesystem::path temp = sourceLocation.file_name();
			result << std::format("{}{}:{} ", GRAY, temp.filename().string(), sourceLocation.line());
		}

		if (prefixLevel >= PrefixLevel::Multithread) {
			result << "(thread: " << std::this_thread::get_id() << ") ";
		}

		result.seekp(-1, result.cur); //"remove" last space 
		result << RESET << ": " << GetLogLevelColour(logLevel);
		return result.str();
	}
}