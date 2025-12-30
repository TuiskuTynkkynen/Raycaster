#include "RenderAPI.h"

#include "Core/Debug/Assert.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace Core {
    void RenderAPI::Init() {
        int32_t success = gladLoadGL(glfwGetProcAddress);
        RC_ASSERT(success, "Failed to initialize GLAD");

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
        glLineWidth(static_cast<GLfloat>(width));
    }

    void RenderAPI::SetDepthBuffer(bool enabled) {
        if (enabled) {
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
        }
    }

    void RenderAPI::SetScissor(bool enabled) {
        if (enabled) {
            glEnable(GL_SCISSOR_TEST);
        } else {
            glDisable(GL_SCISSOR_TEST);
        }
    }

    void RenderAPI::SetScissorRectangle(uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height) {
        glScissor(offsetX, offsetY, width, height);
    }
}
