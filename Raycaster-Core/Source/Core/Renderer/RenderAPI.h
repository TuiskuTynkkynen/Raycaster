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
    enum class DepthFunction : uint8_t {
        Less = 0,
        LessEqual,
        Greater,
        GreaterEqual,
        NotEqual,
        Equal,
        Never,  // Never Draw
        Always, // Always Draw
    };
    void SetDepthFunction(DepthFunction function = DepthFunction::Less); // Only works if depth test is enabled (SetDepthBuffer(true), has been called)

    void SetScissor(bool enabed);
    void SetScissorRectangle(uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height); // Only works if scissor test is enabled (SetScissor(true), has been called)

    int32_t GetMaxMultisampleCount();
    int32_t GetMaxTextureUnitCount();
}

