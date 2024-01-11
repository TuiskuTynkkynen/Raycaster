#include "VertexBufferLayout.h"

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
}