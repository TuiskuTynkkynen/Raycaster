#pragma once
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

#include  <cstdint>
#include  <utility>

namespace Core {
    class VertexArray {
    public:
        VertexArray();
        ~VertexArray();

        VertexArray(const VertexArray&) = delete;
        VertexArray(VertexArray&& other) noexcept
            : m_RendererID(std::exchange(other.m_RendererID, 0)) {};
        VertexArray& operator=(const VertexArray&) = delete;
        VertexArray& operator=(VertexArray&& other) noexcept {
            std::swap(m_RendererID, other.m_RendererID);
            return *this;
        };

        void Bind() const;
        void Unbind() const;

        void AddBuffer(const Core::VertexBuffer& VBO, const Core::VertexBufferLayout& layout) const;
    private:
        uint32_t m_RendererID;
    };
}

