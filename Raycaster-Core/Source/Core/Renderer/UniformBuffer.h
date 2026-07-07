#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <utility>

namespace Core {
    class UniformBuffer {
    public:
        UniformBuffer(size_t size);
        UniformBuffer(std::span<const std::byte> data);
        ~UniformBuffer();

        UniformBuffer(const UniformBuffer&) = delete;
        UniformBuffer(UniformBuffer&& other) noexcept
            : m_Buffer(std::exchange(other.m_Buffer, 0)), m_Size(std::exchange(other.m_Size, 0)) {};
        UniformBuffer& operator=(const UniformBuffer&) = delete;
        UniformBuffer& operator=(UniformBuffer&& other) noexcept {
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
