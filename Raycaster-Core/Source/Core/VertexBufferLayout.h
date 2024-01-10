#pragma once

#include <glfw/glfw3.h>

#include  <vector>

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

		template<>
		void Push<float>(uint32_t count) {
			m_Pointers.push_back({GL_FLOAT, count, GL_FALSE});
			m_Stride += count * VertexAttributePointer::GetSizeOfType(GL_FLOAT);
		}

		template<>
		void Push<uint32_t>(uint32_t count) {
			m_Pointers.push_back({GL_UNSIGNED_INT, count, GL_FALSE});
			m_Stride += count * VertexAttributePointer::GetSizeOfType(GL_UNSIGNED_INT);
		}

		template<typename T>
		void Push(uint32_t count, uint32_t stride);

		template<>
		void Push<float>(uint32_t count, uint32_t stride) {
			m_Pointers.push_back({ GL_FLOAT, count, GL_FALSE });
			m_Stride += stride * VertexAttributePointer::GetSizeOfType(GL_FLOAT);
		}

		template<>
		void Push<uint32_t>(uint32_t count, uint32_t stride) {
			m_Pointers.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
			m_Stride += stride * VertexAttributePointer::GetSizeOfType(GL_UNSIGNED_INT);
		}


		inline const std::vector<VertexAttributePointer>& GetAttributes() const { return m_Pointers; }
		inline uint32_t GetStride() const { return m_Stride;  }
	};
}

