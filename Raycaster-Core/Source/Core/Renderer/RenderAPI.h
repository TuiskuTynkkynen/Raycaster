#pragma once

#include "VertexArray.h"
#include "ElementBuffer.h"

#include <glm/glm.hpp>

namespace Core::RenderAPI {
    void Init();

    void SetViewPort(uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height);
    void SetClearColour(const glm::vec3& colour);
    void SetClearColour(float r, float g, float b);
    void Clear();

    void DrawVertices(const VertexArray& array, uint32_t vertexCount);
    void DrawIndexed(const VertexArray& array, uint32_t indexCount);
    void DrawLines(const VertexArray& array, uint32_t vertexCount);

    void SetLineWidth(uint32_t width);
    void SetDepthBuffer(bool enabled);
}

