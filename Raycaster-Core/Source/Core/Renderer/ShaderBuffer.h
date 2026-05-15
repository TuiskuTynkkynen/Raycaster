#pragma once

#include "Platform.h"

#include <cstdint>
#include <cstddef>
#include <span>

namespace Core {
    class ShaderStorageBuffer {
    public:
#if defined(PLATFORM_EMSCRIPTEN) // SSBO are not supported in web builds
        ShaderStorageBuffer(size_t size) = delete;
        ShaderStorageBuffer(std::span<const std::byte> data) = delete;
#else
        ShaderStorageBuffer(size_t size);
        ShaderStorageBuffer(std::span<const std::byte> data);
#endif
        ~ShaderStorageBuffer();

        void Update(std::span<const std::byte> data, size_t offset = 0);

        void Bind(uint32_t bufferIndex);
    private:
        uint32_t m_Buffer;
        size_t m_Size;
    };
}
