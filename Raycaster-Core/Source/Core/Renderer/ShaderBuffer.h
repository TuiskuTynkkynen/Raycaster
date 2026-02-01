#pragma once

#include <cstdint>
#include <span>

namespace Core {
    class ShaderStorageBuffer {
    public:
        ShaderStorageBuffer(size_t size);
        ShaderStorageBuffer(std::span<const std::byte> data);
        ~ShaderStorageBuffer();

        void Update(std::span<const std::byte> data, size_t offset = 0);

        void Bind(uint32_t bufferIndex);
    private:
        uint32_t m_Buffer;
        size_t m_Size;
    };
}