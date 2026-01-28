#include "RenderAPI.h"

#include "Core/Debug/Debug.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

struct ContextConstants {
    int MultiSampleCount;
    int TextureUnitCount;
} static s_Constants;

static void OpenGLDebugCallback(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*, const void*);

namespace Core {
    void RenderAPI::Init() {
        int32_t success = gladLoadGL(glfwGetProcAddress);
        RC_ASSERT(success, "Failed to initialize GLAD");

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_LINE_WIDTH);


        glGetIntegerv(GL_MAX_SAMPLES, &s_Constants.MultiSampleCount);
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &s_Constants.TextureUnitCount);

#ifdef RC_DEBUG_MODE
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(OpenGLDebugCallback, nullptr);

    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    glDebugMessageControl(GL_DEBUG_SOURCE_THIRD_PARTY, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_TRUE);
    glDebugMessageControl(GL_DEBUG_SOURCE_APPLICATION, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_TRUE);
#endif 
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


    int32_t RenderAPI::GetMaxMultisampleCount() {
        return s_Constants.MultiSampleCount;
    }

    int32_t RenderAPI::GetMaxTextureUnitCount() {
        return s_Constants.TextureUnitCount;
    }
}

static constexpr std::string_view OpenGLMessageSource(GLenum source) {
    switch (source) {
    case GL_DEBUG_SOURCE_API:
        return "API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        return "WINDOW SYSTEM";
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        return "SHADER COMPILER";
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        return "THIRD PARTY";
    case GL_DEBUG_SOURCE_APPLICATION:
        return "APPLICATION";
    case GL_DEBUG_SOURCE_OTHER:
        return "SOURCE OTHER";
    default:
        return "SOURCE INVALID";
    }
}

static constexpr std::string_view OpenGLMessageType(GLenum type) {
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        return "ERROR";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        return "DEPRECATED BEHAVIOR";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        return "UNDEFINED BEHAVIOR";
    case GL_DEBUG_TYPE_PORTABILITY:
        return "PORTABILITY";
    case GL_DEBUG_TYPE_PERFORMANCE:
        return "PERFORMANCE";
    case GL_DEBUG_TYPE_MARKER:
        return "MARKER";
    case GL_DEBUG_TYPE_PUSH_GROUP:
        return "GROUP PUSH";
    case GL_DEBUG_TYPE_POP_GROUP:
        return "GROUP POP";
    case GL_DEBUG_TYPE_OTHER:
        return "TYPE OTHER";
    default:
        return "TYPE INVALID";
    }
}

static constexpr LogLevel OpenGLMessageSeverity(GLenum severity) {
    switch (severity) {
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        return LogLevel::Trace;
    case GL_DEBUG_SEVERITY_LOW:
        return LogLevel::Info;
    case GL_DEBUG_SEVERITY_MEDIUM:
        return LogLevel::Warn;
    case GL_DEBUG_SEVERITY_HIGH:
        return LogLevel::Error;
    default:
        return LogLevel::Error;
    }
}

static void OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    constexpr PrefixLevel prefix = static_cast<PrefixLevel>(std::min(static_cast<std::underlying_type_t<PrefixLevel>>(PrefixLevel::Reduced), LOG_PREFIX_LEVEL));
    Core::Logger::Log(OpenGLMessageSeverity(severity), prefix, std::source_location::current(), "(OPENGL {} {}) {}", OpenGLMessageSource(source), OpenGLMessageType(type), message);
}