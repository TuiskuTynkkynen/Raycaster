#include "ElementBuffer.h"

#include <glad/gl.h>

namespace Core {
	ElementBuffer::ElementBuffer(const uint32_t* data, uint32_t count)
		: m_RendererID(0), m_Count(count)
	{
		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), data, GL_STATIC_DRAW);
	}

	ElementBuffer::ElementBuffer(uint32_t count)
		: m_RendererID(0), m_Count(0)
	{
		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
	}

	ElementBuffer::~ElementBuffer() {
		glDeleteBuffers(1, &m_RendererID);
	}

	void ElementBuffer::Bind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void ElementBuffer::Unbind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void ElementBuffer::SetData(const uint32_t* data, uint32_t count) {
		m_Count = count;

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, count * sizeof(uint32_t), data);
	}
}