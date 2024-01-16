#pragma once

#include "VertexArray.h"
#include "ElementBuffer.h"

#include <glm/glm.hpp>

namespace Core {
	static class RenderAPI {
	public:
		static void Init();

		static void SetViewPort(uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height);
		static void SetClearColour(glm::vec3& colour);
		static void SetClearColour(float r, float g, float b);
		static void Clear();

		static void DrawVertices(VertexArray& array, uint32_t vertexCount);
		static void DrawIndexed(VertexArray& array, uint32_t indexCount);
		static void DrawLines(VertexArray& array, uint32_t vertexCount);

		static void SetLineWidth(uint32_t width);
		static void SetDepthBuffer(bool enabled);
	};
}

