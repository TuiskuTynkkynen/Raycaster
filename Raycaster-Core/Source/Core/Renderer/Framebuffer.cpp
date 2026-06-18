#include "Framebuffer.h"

#include "RenderAPI.h"
#include "VertexArray.h"
#include "Core/Debug/Debug.h"

#if !defined(PLATFORM_EMSCRIPTEN)
    #include <glad/gl.h>
#else
    #include <GLES3/gl3.h>
#endif

#include <utility>

struct FullsceenQuad {
    Core::VertexBuffer VBO;
    Core::VertexArray VAO;

    FullsceenQuad(const void* vertices, uint32_t size, Core::VertexBufferLayout layout) : VBO(vertices, size) {
        VAO.AddBuffer(VBO, layout);
    }
} static* s_RenderQuad = nullptr;

namespace Core {
    void Framebuffer::InitRender() {
        float quadVertices[] = {
                -1.0f,  1.0f,  0.0f, 1.0f,
                -1.0f, -1.0f,  0.0f, 0.0f,
                 1.0f,  1.0f,  1.0f, 1.0f,
                 1.0f, -1.0f,  1.0f, 0.0f,
        };

        VertexBufferLayout quadLayout;
        quadLayout.Push<float>(2);
        quadLayout.Push<float>(2);

        s_RenderQuad = new FullsceenQuad(quadVertices, sizeof(quadVertices), quadLayout);
    }

    void Framebuffer::ShutdownRender() {
        delete s_RenderQuad;
        s_RenderQuad = nullptr;
        return;
    }

    static constexpr GLenum GetInternalFormat(Framebuffer::ColorFormat format) {
        switch (format) {
            using enum Framebuffer::ColorFormat;
        case HDR_F: return GL_R11F_G11F_B10F;
        case RGBA8: return GL_RGBA8;
        case RG8: return GL_RG8;
        case R8: return GL_R8;
#if !defined(PLATFORM_EMSCRIPTEN)
        case RGBA16: return GL_RGBA16;
        case RG16: return GL_RG16;
        case R16: return GL_R16;
#endif
        case RGBA16F: return GL_RGBA16F;
        case RG16F: return GL_RG16F;
        case R16F: return GL_R16F;
        case RGBA32F: return GL_RGBA32F;
        case RG32F: return GL_RG32F;
        case R32F: return GL_R32F;
        }

        RC_ASSERT(false); // This should never be reached
        return 0;
    }

   static constexpr GLenum GetFormat(Framebuffer::ColorFormat format) {
        switch (format) {
            using enum Framebuffer::ColorFormat;
        case HDR_F: return GL_RGB;
#if !defined(PLATFORM_EMSCRIPTEN)
        case R16:  return GL_RED;
        case RG16: return GL_RG;
        case RGBA16: return GL_RGBA;
#endif
        case R8:
        case R16F:
        case R32F:
            return GL_RED;
        case RG8:
        case RG16F:
        case RG32F:
            return GL_RG;
        case RGBA8:
        case RGBA16F:
        case RGBA32F:
            return GL_RGBA;
        }

        RC_ASSERT(false); // This should never be reached
        return 0;
    }

   static constexpr GLenum GetType(Framebuffer::ColorFormat format) {
        switch (format) {
            using enum Framebuffer::ColorFormat;
        case RGBA8:
        case RG8:
        case R8:
            return GL_UNSIGNED_BYTE;
#if !defined(PLATFORM_EMSCRIPTEN)
        case RGBA16:
        case RG16:
        case R16:
            return GL_UNSIGNED_SHORT;
#endif
        case RG16F:
        case R16F:
        case RGBA16F:
        case RGBA32F:
        case RG32F:
        case R32F:
        case HDR_F:
            return GL_FLOAT;
        }

        RC_ASSERT(false); // This should never be reached
        return 0;
   }

    static uint32_t CreateAndAttachColor(uint32_t internalFramebuffer, uint32_t width, uint32_t height, Framebuffer::ColorFormat format, Framebuffer::ColorFilter filtering) {
        uint32_t color;

        glGenTextures(1, &color);
        glBindTexture(GL_TEXTURE_2D, color);
        glTexImage2D(GL_TEXTURE_2D, 0, GetInternalFormat(format), width, height, 0, GetFormat(format), GetType(format), nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering == Framebuffer::ColorFilter::Nearest ? GL_NEAREST : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering == Framebuffer::ColorFilter::Nearest ? GL_NEAREST : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, internalFramebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        return color;
    }

    static uint32_t CreateAndAttachDepthStencil(uint32_t internalFramebuffer, uint32_t width, uint32_t height) {
        uint32_t depthStencil;

        glGenRenderbuffers(1, &depthStencil);
        glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

        glBindFramebuffer(GL_FRAMEBUFFER, internalFramebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencil);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        return depthStencil;
    }

    Framebuffer::Framebuffer(uint32_t width, uint32_t height, ColorFormat format, ColorFilter filtering, bool hasDepthStencil)
        : m_Size(width, height), m_Format(format), m_Filter(filtering) {
        glGenFramebuffers(1, &m_Buffer);
        m_Color = CreateAndAttachColor(m_Buffer, width, height, format, filtering);

        if (hasDepthStencil) {
            m_DepthStencil = CreateAndAttachDepthStencil(m_Buffer, width, height);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, m_Buffer);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            RC_WARN("Framebuffer creation failed.");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    Framebuffer::~Framebuffer() {
        glDeleteTextures(1, &m_Color);
        glDeleteRenderbuffers(1, &m_DepthStencil);
        glDeleteFramebuffers(1, &m_Buffer);
    }

    void Framebuffer::Resize(uint32_t width, uint32_t height) {
        m_Size = glm::uvec2(width, height);

        uint32_t oldColor = m_Color;
        m_Color = CreateAndAttachColor(m_Buffer, width, height, m_Format, m_Filter);
        glDeleteTextures(1, &oldColor);

        if (m_DepthStencil) {
            uint32_t oldDepthstencil = m_DepthStencil;
            m_DepthStencil = CreateAndAttachDepthStencil(m_Buffer, width, height);
            glDeleteRenderbuffers(1, &oldDepthstencil);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, m_Buffer);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            RC_WARN("Framebuffer resize with width = {}, and  height = {} failed.", width, height);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::Activate() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_Buffer);
    }

    void Framebuffer::Deactivate() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::Blit(glm::uvec2 outputOffset, glm::uvec2 outputSize) {
        Blit({ 0, 0 }, m_Size, outputOffset, outputSize);
    }

    void Framebuffer::Blit(glm::uvec2 offset, glm::uvec2 size, glm::uvec2 outputOffset, glm::uvec2 outputSize) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_Buffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(offset.x, offset.y, offset.x + size.x, offset.y + size.y, outputOffset.x, outputOffset.y, outputOffset.x + outputSize.x, outputOffset.y + outputSize.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::BindToTextureUnit(uint8_t textureUnitIndex) {
        if (std::cmp_greater_equal(textureUnitIndex, RenderAPI::GetMaxTextureUnitCount())) {
            RC_WARN("Texture unit index must be less than or equal to RenderAPI::GetMaxTextureUnitCount");
            return;
        }

        glActiveTexture(GL_TEXTURE0 + textureUnitIndex);
        glBindTexture(GL_TEXTURE_2D, m_Color);
    }

    void Framebuffer::Render(Shader& shader, uint8_t textureUnitIndex) {
        RC_ASSERT(s_RenderQuad, "Framebuffer::InitRender must be called before trying to render a Framebuffer");

        shader.Bind();
        BindToTextureUnit(textureUnitIndex);
        s_RenderQuad->VAO.Bind();

        RenderAPI::SetDepthBuffer(false);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        RenderAPI::SetDepthBuffer(true);
    }

    static uint32_t CreateAndAttachMultisampleColor(uint32_t internalFramebuffer, uint32_t width, uint32_t height, uint8_t sampleCount, Framebuffer::ColorFormat format) {
        uint32_t buffer;
        glGenRenderbuffers(1, &buffer);
        glBindRenderbuffer(GL_RENDERBUFFER, buffer);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, sampleCount, GetInternalFormat(format), width, height);

        glBindFramebuffer(GL_FRAMEBUFFER, internalFramebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, buffer);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        return buffer;
    }

    static uint32_t CreateAndAttachMultisampleDepthStencil(uint32_t internalFramebuffer, uint32_t width, uint32_t height, uint8_t sampleCount) {
        uint32_t buffer;
        glGenRenderbuffers(1, &buffer);
        glBindRenderbuffer(GL_RENDERBUFFER, buffer);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, sampleCount, GL_DEPTH24_STENCIL8, width, height);

        glBindFramebuffer(GL_FRAMEBUFFER, internalFramebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        return buffer;
    }


    MultisampleFramebuffer::MultisampleFramebuffer(uint32_t width, uint32_t height, uint8_t sampleCount, MultisampleFramebuffer::ColorFormat format, Framebuffer::ColorFilter filtering, bool hasDepthStencil)
        : m_Resolved(width, height, static_cast<Framebuffer::ColorFormat>(format), filtering, false) {
        if (!sampleCount || sampleCount > RenderAPI::GetMaxMultisampleCount()) {
            RC_ERROR("Multisample Framebuffer sample count must be greater than 0 and less than or equal to RenderAPI::GetMaxMultisampleCount");
        }

        glGenFramebuffers(1, &m_Buffer);
        m_Color = CreateAndAttachMultisampleColor(m_Buffer, width, height, sampleCount, m_Resolved.GetFormat());

        if (hasDepthStencil) {
            m_DepthStencil = CreateAndAttachMultisampleDepthStencil(m_Buffer, width, height, sampleCount);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, m_Buffer);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            RC_WARN("Multisample Framebuffer creation failed.");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void MultisampleFramebuffer::Resize(uint32_t width, uint32_t height, uint8_t sampleCount) {
        if (!sampleCount || sampleCount > RenderAPI::GetMaxMultisampleCount()) {
            RC_ERROR("Multisample Framebuffer sample count must be greater than 0 and less than or equal to RenderAPI::GetMaxMultisampleCount");
            return;
        }

        m_Resolved.Resize(width, height);

        uint32_t oldColor = m_Color;
        m_Color = CreateAndAttachMultisampleColor(m_Buffer, width, height, sampleCount, m_Resolved.GetFormat());
        glDeleteRenderbuffers(1, &oldColor);

        if (m_DepthStencil) {
            uint32_t oldDepthStencil = m_DepthStencil;
            m_DepthStencil = CreateAndAttachMultisampleDepthStencil(m_Buffer, width, height, sampleCount);
            glDeleteRenderbuffers(1, &oldDepthStencil);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, m_Buffer);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            RC_WARN("Multisample Framebuffer creation failed.");
            RC_WARN("Multisample Framebuffer resize with width = {}, and  height = {} failed.", width, height);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    MultisampleFramebuffer::~MultisampleFramebuffer() {
        glDeleteTextures(1, &m_Color);
        glDeleteRenderbuffers(1, &m_DepthStencil);
        glDeleteFramebuffers(1, &m_Buffer);
    }

    void MultisampleFramebuffer::Activate() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_Buffer);
    }

    void MultisampleFramebuffer::Deactivate() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void MultisampleFramebuffer::Blit(glm::uvec2 outputOffset, glm::uvec2 outputSize) {
        Blit({ 0, 0 }, m_Resolved.GetSize(), outputOffset, outputSize);
    }

    void MultisampleFramebuffer::Blit(glm::uvec2 offset, glm::uvec2 size, glm::uvec2 outputOffset, glm::uvec2 outputSize) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_Buffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Resolved.GetBuffer());
        glBlitFramebuffer(offset.x, offset.y, offset.x + size.x, offset.y + size.y, offset.x, offset.y, offset.x + size.x, offset.y + size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_Resolved.GetBuffer());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(offset.x, offset.y, offset.x + size.x, offset.y + size.y, outputOffset.x, outputOffset.y, outputOffset.x + outputSize.x, outputOffset.y + outputSize.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void MultisampleFramebuffer::BindToTextureUnit(uint8_t textureUnitIndex) {
        auto size = m_Resolved.GetSize();
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_Buffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Resolved.GetBuffer());
        glBlitFramebuffer(0, 0, size.x, size.y, 0, 0, size.x, size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        m_Resolved.BindToTextureUnit(textureUnitIndex);
    }

    void MultisampleFramebuffer::Render(Shader& shader, uint8_t textureUnitIndex) {
        RC_ASSERT(s_RenderQuad, "Framebuffer::InitRender must be called before trying to render a MultisampleFramebuffer");

        auto size = m_Resolved.GetSize();
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_Buffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Resolved.GetBuffer());
        glBlitFramebuffer(0, 0, size.x, size.y, 0, 0, size.x, size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        m_Resolved.Render(shader, textureUnitIndex);
    }
}
