#pragma once

#include "VertexArray.h"
#include "ElementBuffer.h"

#include <glm/glm.hpp>

namespace Core {
	class RenderAPI {
	public:
		RenderAPI();

		void SetViewPort(uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height);
		void SetClearColour(glm::vec3& colour);
		void SetClearColour(float r, float g, float b);
		void Clear();

		void DrawVertices(VertexArray& array, uint32_t vertexCount);
		void DrawIndexed(VertexArray& array, uint32_t indexCount);
		void DrawLines(VertexArray& array, uint32_t vertexCount);

		void SetLineWidth(uint32_t width);
		void SetDepthBuffer(bool enabled);
	};
}

