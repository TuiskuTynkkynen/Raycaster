#include "UniformBuffer.h"

#include "Core/Debug/Debug.h"

#include "Platform.h"
#if !defined(PLATFORM_EMSCRIPTEN)
    #include <glad/gl.h>
#else
    #include <GLES3/gl3.h>
#endif

namespace Core {
    UniformBuffer::UniformBuffer(size_t size)
        : m_Size(size) {
        glGenBuffers(1, &m_Buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, m_Buffer);
        glBufferData(GL_UNIFORM_BUFFER, m_Size, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    UniformBuffer::UniformBuffer(std::span<const std::byte> data)
        : m_Size(data.size()) {
        glGenBuffers(1, &m_Buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, m_Buffer);
        glBufferData(GL_UNIFORM_BUFFER, m_Size, data.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    UniformBuffer::~UniformBuffer() {
        glDeleteBuffers(1, &m_Buffer);
    }

    void UniformBuffer::Update(std::span<const std::byte> data, size_t offset) {
        if (offset >= m_Size) {
            RC_ERROR("Tried at offset {} to a UniformBuffer with size {}.", offset, m_Size);
            return;
        }

        size_t size = data.size();
        if (offset + size > m_Size) {
            size = m_Size - offset;
            RC_WARN("Tried to write {} bytes at offset {} to a UniformBuffer with size {}. Writing {} bytes instead", data.size(), offset, m_Size, size);
        }

        glBindBuffer(GL_UNIFORM_BUFFER, m_Buffer);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data.data());
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void UniformBuffer::Bind(uint32_t bufferIndex) {
        glBindBufferBase(GL_UNIFORM_BUFFER, bufferIndex, m_Buffer);
    }
}
