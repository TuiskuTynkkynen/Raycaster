#include "VertexArray.h"

#include <glad/glad.h>

namespace Core {
	VertexArray::VertexArray()
		: m_RendererID(0)
	{
		glGenVertexArrays(1, &m_RendererID);
	}

	VertexArray::~VertexArray() {
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void VertexArray::Bind() const{
		glBindVertexArray(m_RendererID);
	}

	void VertexArray::Unbind() const{
		glBindVertexArray(0);
	}

	void VertexArray::AddBuffer(const Core::VertexBuffer& VBO, const Core::VertexBufferLayout& layout) {
		Bind();
		VBO.Bind();
		const auto& vertexAttributes = layout.GetAttributes();
		uint32_t  offset = 0;

		for (int i = 0; i < vertexAttributes.size(); i++) {
			const VertexAttributePointer& attribute = vertexAttributes[i];

			glVertexAttribPointer(i, attribute.count, attribute.type, attribute.normalized, layout.GetStride(), (void*)offset);
			glEnableVertexAttribArray(i);
			
			offset += attribute.count * VertexAttributePointer::GetSizeOfType(attribute.type);
		}

	}
}