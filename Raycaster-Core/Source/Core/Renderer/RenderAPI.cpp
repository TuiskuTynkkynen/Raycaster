#include "RenderAPI.h"

namespace Core {
    void RenderAPI::Init() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_LINE_WIDTH);
    }

    void RenderAPI::SetViewPort(uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height) {
        glViewport(offsetX, offsetY, width, height);
    }

    void RenderAPI::SetClearColour(const glm::vec3& colour) {
        glClearColor(colour.r, colour.g, colour.b, 1.0f);
    }

    void RenderAPI::SetClearColour(float r, float g, float b) {
        glClearColor(r, g, b, 1.0f);
    }

    void RenderAPI::Clear() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void RenderAPI::DrawVertices(const VertexArray& array, uint32_t vertexCount) {
        array.Bind();
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    void RenderAPI::DrawIndexed(const VertexArray& array, uint32_t indexCount) {
        array.Bind();
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }

    void RenderAPI::DrawLines(const VertexArray& array, uint32_t vertexCount) {
        array.Bind();
        glDrawArrays(GL_LINES, 0, vertexCount);
    }

    void RenderAPI::SetLineWidth(uint32_t width) {
        glLineWidth(width);
    }

    void RenderAPI::SetDepthBuffer(bool enabled) {
        if (enabled) {
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
        }
    }
}