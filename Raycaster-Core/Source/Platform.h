#pragma once

#ifdef _WIN32
    #ifdef _WIN64
        #define PLATFORM_WINDOWS
    #else
        #error "Windows x86 Builds not supported"
    #endif
#elif defined(__ANDROID__)
    #error "Android not supported"
#elif defined(__linux__)
    #define PLATFORM_LINUX
#elif defined(__EMSCRIPTEN__)
    #define PLATFORM_EMSCRIPTEN
#else
    #error "Unknown platform"
#endif

#include <filesystem>

namespace Core {
    std::filesystem::path ApplicationDirectory();

    std::wstring UTF8ToWide(const char* string);
    std::wstring UTF8ToWide(std::string_view string);
    std::string WideToUTF8(std::wstring_view string);
}

namespace Core::Platform {
    #if defined(PLATFORM_WINDOWS)
        inline constexpr bool WINDOWS = true;
        inline constexpr bool NATIVE = true;
    #else
        inline constexpr bool Windows = false;
    #endif

    #if defined(PLATFORM_LINUX)
        inline constexpr bool LINUX = true;
        inline constexpr bool NATIVE = true;
    #else
        inline constexpr bool LINUX = false;
    #endif

    #if defined(PLATFORM_EMSCRIPTEN)
        inline constexpr bool EMSCRIPTEN = true;
        inline constexpr bool NATIVE = false;
    #else
        inline constexpr bool EMSCRIPTEN = false;
    #endif

    inline constexpr bool WEB = !NATIVE;
}