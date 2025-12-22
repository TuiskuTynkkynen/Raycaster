#pragma once

#include <vector>
#include <cstdint>

struct VertexAttributePointer{
    uint32_t type;
    uint32_t count;
    unsigned char normalized;

    static uint32_t GetSizeOfType(uint32_t type);
};

namespace Core {
    class VertexBufferLayout
    {
    private:
        std::vector<VertexAttributePointer> m_Pointers;
        uint32_t m_Stride;
    public:
        VertexBufferLayout();

        template<typename T>
        void Push(uint32_t count);

        template<typename T>
        void Push(uint32_t count, uint32_t stride);

        inline const std::vector<VertexAttributePointer>& GetAttributes() const { return m_Pointers; }
        inline uint32_t GetStride() const { return m_Stride;  }
    };
}

