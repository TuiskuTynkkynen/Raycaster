#pragma once

#include "Platform.h"

#include <cstdint>
#include <cstddef>
#include <span>
#include <utility>

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

        ShaderStorageBuffer(const ShaderStorageBuffer&) = delete;
        ShaderStorageBuffer(ShaderStorageBuffer&& other) noexcept
            : m_Buffer(std::exchange(other.m_Buffer, 0)), m_Size(std::exchange(other.m_Size, 0)) {};
        ShaderStorageBuffer& operator=(const ShaderStorageBuffer&) = delete;
        ShaderStorageBuffer& operator=(ShaderStorageBuffer&& other) noexcept {
            std::swap(m_Buffer, other.m_Buffer); std::swap(m_Size, other.m_Size);
            return *this;
        };

        void Update(std::span<const std::byte> data, size_t offset = 0);

        void Bind(uint32_t bufferIndex);
    private:
        uint32_t m_Buffer;
        size_t m_Size;
    };
}
