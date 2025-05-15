#include "VertexBufferLayout.h"

#include <glfw/glfw3.h>

uint32_t VertexAttributePointer::GetSizeOfType(uint32_t type) {
    switch (type)
    {
    case GL_FLOAT:
        return 4;
    case GL_UNSIGNED_INT:
        return 4;
    }
}

namespace Core {
    VertexBufferLayout::VertexBufferLayout()
        : m_Stride(0) {}


    template<>
    void VertexBufferLayout::Push<float>(uint32_t count) {
        m_Pointers.push_back({ GL_FLOAT, count, GL_FALSE });
        m_Stride += count * VertexAttributePointer::GetSizeOfType(GL_FLOAT);
    }

    template<>
    void VertexBufferLayout::Push<uint32_t>(uint32_t count) {
        m_Pointers.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
        m_Stride += count * VertexAttributePointer::GetSizeOfType(GL_UNSIGNED_INT);
    }

    template<>
    void VertexBufferLayout::Push<float>(uint32_t count, uint32_t stride) {
        m_Pointers.push_back({ GL_FLOAT, count, GL_FALSE });
        m_Stride += stride * VertexAttributePointer::GetSizeOfType(GL_FLOAT);
    }

    template<>
    void VertexBufferLayout::Push<uint32_t>(uint32_t count, uint32_t stride) {
        m_Pointers.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
        m_Stride += stride * VertexAttributePointer::GetSizeOfType(GL_UNSIGNED_INT);
    }
}