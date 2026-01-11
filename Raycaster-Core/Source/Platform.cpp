#include "Platform.h"

#include "Core/Debug/Assert.h"

#include <array>

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
    
static std::filesystem::path CreatePath() {
    std::array<wchar_t, MAX_PATH> buffer{};
    size_t length = GetModuleFileName(NULL, buffer.data(), MAX_PATH);

    if (length != 0) {
        return std::filesystem::path(buffer.begin(), buffer.begin() + length).remove_filename();
    }

    return {};
}

std::wstring Core::UTF8ToWide(const char* string) {
    int32_t utf8Size = static_cast<int32_t>(std::strlen(string));
    int32_t wideSize = MultiByteToWideChar(CP_UTF8, 0, string, utf8Size, NULL, 0);

    std::wstring wide(wideSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, string, utf8Size, wide.data(), wideSize);

    return wide;
}

std::wstring Core::UTF8ToWide(std::string_view string) {
    int32_t utf8Size = static_cast<int32_t>(string.size());
    int32_t wideSize = MultiByteToWideChar(CP_UTF8, 0, string.data(), utf8Size, NULL, 0);

    std::wstring wide(wideSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, string.data(), utf8Size, wide.data(), wideSize);

    return wide;
}

std::string Core::WideToUTF8(std::wstring_view string) {
    int32_t wideSize = static_cast<int32_t>(string.size());
    int32_t utf8Size = WideCharToMultiByte(CP_UTF8, 0, string.data(), wideSize, NULL, 0, NULL, NULL);
    
    std::string utf8(utf8Size, 0);
    WideCharToMultiByte(CP_UTF8, 0, string.data(), wideSize, utf8.data(), utf8Size, NULL, NULL);
    
    return utf8;
}

#elif defined(PLATFORM_LINUX)
#include <unistd.h>

static std::filesystem::path CreatePath() {
    std::array<char, PATH_MAX> buffer{};
    ssize_t length = ::readlink("/proc/self/exe", buffer.data(), PATH_MAX);

    if (length != -1) {
        return std::filesystem::path(buffer.begin(), buffer.begin() + length).remove_filename();
    }

    return {};
}

#ifndef __clang__
    #error "Only clang is supported on Linux"
#endif 

#include <codecvt>
#include <locale>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

std::wstring Core::UTF8ToWide(const char* string) {
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(string);
}

std::wstring Core::UTF8ToWide(std::string_view string) {
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(string.data(), string.data() + string.size());
}

std::string Core::WideToUTF8(std::wstring_view string) {
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(string.data());
}

# pragma clang diagnostic pop

#endif

static std::filesystem::path s_Path = CreatePath();

std::filesystem::path Core::ApplicationDirectory() {
    RC_ASSERT(!s_Path.empty(), "Could not get path to application directory.");
    return s_Path;
}
