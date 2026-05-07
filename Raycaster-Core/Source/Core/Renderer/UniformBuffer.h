#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

namespace Core {
    class UniformBuffer {
    public:
        UniformBuffer(size_t size);
        UniformBuffer(std::span<const std::byte> data);
        ~UniformBuffer();

        void Update(std::span<const std::byte> data, size_t offset = 0);

        void Bind(uint32_t bufferIndex);
    private:
        uint32_t m_Buffer;
        size_t m_Size;
    };
}
