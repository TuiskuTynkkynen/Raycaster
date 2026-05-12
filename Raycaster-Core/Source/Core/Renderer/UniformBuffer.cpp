#include "UniformBuffer.h"

#include "Core/Debug/Debug.h"

#include <glad/gl.h>

namespace Core {
    UniformBuffer::UniformBuffer(size_t size)
        : m_Size(size) {
        glCreateBuffers(1, &m_Buffer);
        glNamedBufferStorage(m_Buffer, m_Size, nullptr, GL_DYNAMIC_STORAGE_BIT);
    }

    UniformBuffer::UniformBuffer(std::span<const std::byte> data)
        : m_Size(data.size()) {
        glCreateBuffers(1, &m_Buffer);
        glNamedBufferStorage(m_Buffer, m_Size, data.data(), GL_DYNAMIC_STORAGE_BIT);
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

        glNamedBufferSubData(m_Buffer, offset, size, data.data());
    }

    void UniformBuffer::Bind(uint32_t bufferIndex) {
        glBindBufferBase(GL_UNIFORM_BUFFER, bufferIndex, m_Buffer);
    }
}