#pragma once

#include  <cstdint>
#include  <utility>

namespace Core {
    class VertexBuffer {
    public:
        VertexBuffer(const void* data, uint32_t size);
        VertexBuffer(uint32_t size);
        ~VertexBuffer();

        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer(VertexBuffer&& other) noexcept
            : m_RendererID(std::exchange(other.m_RendererID, 0)) {};
        VertexBuffer& operator=(const VertexBuffer&) = delete;
        VertexBuffer& operator=(VertexBuffer&& other) noexcept {
            std::swap(m_RendererID, other.m_RendererID);
            return *this;
        };

        void Bind() const;
        void Unbind() const;

        void SetData(const void* data, uint32_t size) const;
    private:
        uint32_t m_RendererID;
    };
}

