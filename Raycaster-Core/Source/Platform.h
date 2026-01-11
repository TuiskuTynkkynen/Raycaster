//Platform
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