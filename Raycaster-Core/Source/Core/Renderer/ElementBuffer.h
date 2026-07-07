#pragma once

#include  <cstdint>
#include  <utility>

namespace Core {
    class ElementBuffer {
    public:
        ElementBuffer(const uint32_t* data, uint32_t count); 
        ElementBuffer(uint32_t size);
        ~ElementBuffer();

        ElementBuffer(const ElementBuffer&) = delete;
        ElementBuffer(ElementBuffer&& other) noexcept
            : m_RendererID(std::exchange(other.m_RendererID, 0)), m_Count(std::exchange(other.m_Count, 0)) {};
        ElementBuffer& operator=(const ElementBuffer&) = delete;
        ElementBuffer& operator=(ElementBuffer&& other) noexcept {
            std::swap(m_RendererID, other.m_RendererID); std::swap(m_Count, other.m_Count);
            return *this;
        };

        void Bind() const;
        void Unbind() const;

        void SetData(const uint32_t* data, uint32_t count);

        inline uint32_t GetCount() { return m_Count; }
    private:
        uint32_t m_RendererID;
        uint32_t m_Count;
    };
}

