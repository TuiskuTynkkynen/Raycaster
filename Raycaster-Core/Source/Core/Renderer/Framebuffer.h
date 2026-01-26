#pragma once

#include <glm/glm.hpp>
#include <cstdint>

namespace Core {
    class Framebuffer {
    public:
        enum class ColorFormat : uint8_t {
            RGB8 = 0,
            RG8,
            R8,
            RGB16,
            RG16,
            R16,
            RGB16F,
            RG16F,
            R16F,
            RGB32F,
            RG32F,
            R32F,
        };
        Framebuffer(uint32_t width, uint32_t height, ColorFormat format = ColorFormat::RGB8, bool hasDepthStencil = true);
        ~Framebuffer();

        void Activate();
        void Deactivate();

        void Blit(glm::uvec2 outputOffset, glm::uvec2 outputSize);
        void Blit(glm::uvec2 offset, glm::uvec2 size, glm::uvec2 outputOffset, glm::uvec2 outputSize);

        inline uint32_t GetBuffer() const { return m_Buffer; }
        inline glm::uvec2 GetSize() const { return m_Size; }
    private:
        uint32_t m_Buffer;
        uint32_t m_Color;
        uint32_t m_DepthStencil = 0;

        glm::uvec2 m_Size;
    };

    class MultisampleFramebuffer {
    public:
        MultisampleFramebuffer(uint32_t width, uint32_t height, uint8_t sampleCount, Framebuffer::ColorFormat format = Framebuffer::ColorFormat::RGB8, bool hasDepthStencil = true);
        ~MultisampleFramebuffer();

        void Activate();
        void Deactivate();

        void Blit(glm::uvec2 outputOffset, glm::uvec2 outputSize);
        void Blit(glm::uvec2 offset, glm::uvec2 size, glm::uvec2 outputOffset, glm::uvec2 outputSize);

        inline uint32_t GetBuffer() const { return m_Buffer; }
        inline glm::uvec2 GetSize() const { return m_Resolved.GetSize(); }
    private:
        uint32_t m_Buffer;
        uint32_t m_Color;
        uint32_t m_DepthStencil = 0;

        Framebuffer m_Resolved;
    };
}