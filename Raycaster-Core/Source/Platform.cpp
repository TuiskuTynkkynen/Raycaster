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

#elif defined(PLATFORM_LINUX)
#include <unistd.h>

static std::filesystem::path CreatePath() {
    std::array<char, PATH_MAX> buffer{};
    ssize_t length = ::readlink("/proc/self/exe", buffer.data(), PATH_MAX);

    if (length != -1) {
        return std::filesystem::path(buffer.begin(), buffer.begin() + length);
    }

    return {};
}
#endif

static std::filesystem::path s_Path = CreatePath();

std::filesystem::path Core::ApplicationDirectory() {
    RC_ASSERT(!s_Path.empty(), "Could not get path to application directory.");
    return s_Path;
}