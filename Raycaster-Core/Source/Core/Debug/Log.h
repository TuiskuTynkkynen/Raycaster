#pragma once

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <format>
#include <source_location>
#include <filesystem>
#include <cstdio>

#ifndef LOG_PREFIX_LEVEL
    #define LOG_PREFIX_LEVEL 2 //Default prefix -> Full
#endif

enum class PrefixLevel{
    Minimal = 0,
    Reduced,
    Full,
    Multithread,
};

enum class LogLevel {
    Trace = 0,
    Info,
    Warn,
    Error,
    Fatal,
};

namespace Core {
    class Logger{
    private:
        static constexpr const char* GetLogLevelString(LogLevel logLevel);
        static constexpr const char* GetLogLevelColour(LogLevel logLevel);
        static std::string CreatePrefix(LogLevel logLevel, PrefixLevel prefixLevel, const std::source_location& sourceLocation);
    public:
        template <typename ... Args>
        static void Log(LogLevel logLevel, PrefixLevel prefixLevel, const std::source_location location, const std::format_string<Args...> fmt, Args&&... args) {
            constexpr auto RESET = "\033[0m";
            std::string temp = std::format("{}{}{}", CreatePrefix(logLevel, prefixLevel, location), std::vformat(fmt.get(), std::make_format_args(args...)), RESET);
            std::puts(temp.c_str());
        }
    };
    
}

#ifdef LOG_ENABLE
    #define RC_TRACE(...) Core::Logger::Log(LogLevel::Trace, (PrefixLevel)LOG_PREFIX_LEVEL, std::source_location::current(), __VA_ARGS__)
    #define RC_INFO(...) Core::Logger::Log(LogLevel::Info, (PrefixLevel)LOG_PREFIX_LEVEL, std::source_location::current(), __VA_ARGS__)
    #define RC_WARN(...) Core::Logger::Log(LogLevel::Warn, (PrefixLevel)LOG_PREFIX_LEVEL, std::source_location::current(), __VA_ARGS__)
    #define RC_ERROR(...) Core::Logger::Log(LogLevel::Error, (PrefixLevel)LOG_PREFIX_LEVEL, std::source_location::current(), __VA_ARGS__)
    #define RC_FATAL(...) Core::Logger::Log(LogLevel::Fatal, (PrefixLevel)LOG_PREFIX_LEVEL, std::source_location::current(), __VA_ARGS__)
#else
    #define RC_TRACE(...) 
    #define RC_INFO(...) 
    #define RC_WARN(...) 
    #define RC_ERROR(...) 
    #define RC_FATAL(...) 
#endif


template <glm::length_t D, typename T, glm::qualifier Q>
struct std::formatter<glm::vec<D, T, Q>>
{
    auto format(const glm::vec<D, T, Q>& value, std::format_context& context) const {
        return format_to(context.out(), "{}", glm::to_string(value));
    }

    constexpr auto parse(std::format_parse_context& context){
        return context.end();
    }
};

template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct std::formatter<glm::mat<C, R, T, Q>> 
{
    auto format(const glm::mat<C, R, T, Q>& value, std::format_context& context) const{
        return format_to(context.out(), "{}", glm::to_string(value));
    }

    constexpr auto parse(std::format_parse_context& context){
        return context.end();
    }
};

template<>
struct std::formatter<std::source_location> {
    auto format(const std::source_location& value, std::format_context& context) const {
        std::filesystem::path temp = value.file_name();
        return format_to(context.out(), "{}:{}", temp.filename().string(), value.line());
    }

    constexpr auto parse(std::format_parse_context& context) {
        return context.end();
    }
};