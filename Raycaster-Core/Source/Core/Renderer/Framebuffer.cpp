#include "Framebuffer.h"

#include "RenderAPI.h"
#include "VertexArray.h"
#include "Core/Debug/Debug.h"

#include <glad/gl.h>

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
        case RGB8: return GL_RGB8;
        case RG8: return GL_RG8;
        case R8: return GL_R8;
        case RGB16: return GL_RGB16;
        case RG16: return GL_RG16;
        case R16: return GL_R16;
        case RGB16F: return GL_RGB16F;
        case RG16F: return GL_RG16F;
        case R16F: return GL_R16F;
        case RGB32F: return GL_RGB32F;
        case RG32F: return GL_RG32F;
        case R32F: return GL_R32F;    
        }

        RC_ASSERT(false); // This should never be reached
        return 0;
    }

    static uint32_t CreateAndAttachColor(uint32_t internalFramebuffer, uint32_t width, uint32_t height, Framebuffer::ColorFormat format) {
        uint32_t color;
        glCreateTextures(GL_TEXTURE_2D, 1, &color);
        glTextureStorage2D(color, 1, GetInternalFormat(format), width, height);

        glTextureParameteri(color, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(color, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(color, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(color, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glNamedFramebufferTexture(internalFramebuffer, GL_COLOR_ATTACHMENT0, color, 0);
        return color;
    }

    static uint32_t CreateAndAttachDepthStencil(uint32_t internalFramebuffer, uint32_t width, uint32_t height) {
        uint32_t depthStencil;
        glCreateRenderbuffers(1, &depthStencil);
        glNamedRenderbufferStorage(depthStencil, GL_DEPTH24_STENCIL8, width, height);
        glNamedFramebufferRenderbuffer(internalFramebuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencil);
        return depthStencil;
    }

    Framebuffer::Framebuffer(uint32_t width, uint32_t height, ColorFormat format, bool hasDepthStencil) 
        : m_Size(width, height), m_Format(format) {
        
        glCreateFramebuffers(1, &m_Buffer);
        m_Color = CreateAndAttachColor(m_Buffer, width, height, format);
        
        if (hasDepthStencil) {
            m_DepthStencil = CreateAndAttachDepthStencil(m_Buffer, width, height);
        }

        if (glCheckNamedFramebufferStatus(m_Buffer, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            RC_WARN("Framebuffer creation failed.");
        }
    }

    Framebuffer::~Framebuffer() {
        glDeleteTextures(1, &m_Color);
        glDeleteRenderbuffers(1, &m_DepthStencil);
        glDeleteFramebuffers(1, &m_Buffer);
    }

    void Framebuffer::Resize(uint32_t width, uint32_t height) {
        m_Size = glm::uvec2(width, height);

        uint32_t oldColor = m_Color;
        m_Color = CreateAndAttachColor(m_Buffer, width, height, m_Format);
        glDeleteTextures(1, &oldColor);

        if (m_DepthStencil) {
            uint32_t oldDepthstencil = m_DepthStencil;
            m_DepthStencil = CreateAndAttachDepthStencil(m_Buffer, width, height);
            glDeleteRenderbuffers(1, &oldDepthstencil);
        }

        if (glCheckNamedFramebufferStatus(m_Buffer, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            RC_WARN("Framebuffer resize with width = {}, and  height = {} failed.", width, height);
        }
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
        glBlitNamedFramebuffer(m_Buffer, 0, offset.x, offset.y, offset.x + size.x, offset.y + size.y, outputOffset.x, outputOffset.y, outputOffset.x + outputSize.x, outputOffset.y + outputSize.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    void Framebuffer::BindToTextureUnit(uint8_t textureUnitIndex) {
        if (std::cmp_greater_equal(textureUnitIndex, RenderAPI::GetMaxTextureUnitCount())) {
            RC_WARN("Texture unit index must be less than or equal to RenderAPI::GetMaxTextureUnitCount");
            return;
        }

        glBindTextureUnit(textureUnitIndex, m_Color);
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
        glCreateRenderbuffers(1, &buffer);
        glNamedRenderbufferStorageMultisample(buffer, sampleCount, GetInternalFormat(format), width, height);
        glNamedFramebufferRenderbuffer(internalFramebuffer, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, buffer);
        return buffer;
    }

    static uint32_t CreateAndAttachMultisampleDepthStencil(uint32_t internalFramebuffer, uint32_t width, uint32_t height, uint8_t sampleCount) {
        uint32_t buffer;
        glCreateRenderbuffers(1, &buffer);
        glNamedRenderbufferStorageMultisample(buffer, sampleCount, GL_DEPTH24_STENCIL8, width, height);
        glNamedFramebufferRenderbuffer(internalFramebuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer);
        return buffer;
    }


    MultisampleFramebuffer::MultisampleFramebuffer(uint32_t width, uint32_t height, uint8_t sampleCount, Framebuffer::ColorFormat format, bool hasDepthStencil)
        : m_Resolved(width, height, format, false) {
        if (!sampleCount || sampleCount > RenderAPI::GetMaxMultisampleCount()) {
            RC_ERROR("Multisample Framebuffer sample count must be greater than 0 and less than or equal to RenderAPI::GetMaxMultisampleCount");
        }

        glCreateFramebuffers(1, &m_Buffer);
        m_Color = CreateAndAttachMultisampleColor(m_Buffer, width, height, sampleCount, format);
        
        if (hasDepthStencil) {
            m_DepthStencil = CreateAndAttachMultisampleDepthStencil(m_Buffer, width, height, sampleCount);
        }

        if (glCheckNamedFramebufferStatus(m_Buffer, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            RC_WARN("Multisample Framebuffer creation failed.");
        }
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

        if (glCheckNamedFramebufferStatus(m_Buffer, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            RC_WARN("Multisample Framebuffer resize with width = {}, and  height = {} failed.", width, height);
        }
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
        glBlitNamedFramebuffer(m_Buffer, m_Resolved.GetBuffer(), offset.x, offset.y, offset.x + size.x, offset.y + size.y, offset.x, offset.y, offset.x + size.x, offset.y + size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBlitNamedFramebuffer(m_Resolved.GetBuffer(), 0, offset.x, offset.y, offset.x + size.x, offset.y + size.y, outputOffset.x, outputOffset.y, outputOffset.x + outputSize.x, outputOffset.y + outputSize.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    void MultisampleFramebuffer::BindToTextureUnit(uint8_t textureUnitIndex) {
        auto size = m_Resolved.GetSize();
        glBlitNamedFramebuffer(m_Buffer, m_Resolved.GetBuffer(), 0, 0, size.x, size.y, 0, 0, size.x, size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        
        m_Resolved.BindToTextureUnit(textureUnitIndex);
    }

    void MultisampleFramebuffer::Render(Shader& shader, uint8_t textureUnitIndex) {
        RC_ASSERT(s_RenderQuad, "Framebuffer::InitRender must be called before trying to render a MultisampleFramebuffer");

        auto size = m_Resolved.GetSize();
        glBlitNamedFramebuffer(m_Buffer, m_Resolved.GetBuffer(), 0, 0, size.x, size.y, 0, 0, size.x, size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        m_Resolved.Render(shader, textureUnitIndex);
    }
}