#pragma once
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

#include  <cstdint>

namespace Core {
	class VertexArray
	{
	private:
		uint32_t m_RendererID;
	public:
		VertexArray();
		~VertexArray();

		void Bind() const;
		void Unbind() const;

		void AddBuffer(const Core::VertexBuffer& VBO, const Core::VertexBufferLayout& layout);
	};
}

