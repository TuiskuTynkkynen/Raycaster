#pragma once

#include "Shader.h"
#include "Platform.h"

#include <glm/glm.hpp>
#include <cstdint>

namespace Core {
    class Framebuffer {
    public:
        enum class ColorFormat : uint8_t {
            HDR_F = 255,
            RGBA8 = 0,
            RG8,
            R8,
#if !defined(PLATFORM_EMSCRIPTEN)
            RGBA16,
            RG16,
            R16,
#endif
            RGBA16F,
            RG16F,
            R16F,
            RGBA32F,
            RG32F,
            R32F,
        };
        enum class ColorFilter : uint8_t {
            Nearest = 0,
            Linear,
        };
        Framebuffer(uint32_t width, uint32_t height, ColorFormat format = ColorFormat::RGBA8, ColorFilter filtering = ColorFilter::Nearest, bool hasDepthStencil = true);
        ~Framebuffer();

        void Resize(uint32_t width, uint32_t height);

        void Activate();
        void Deactivate();

        void Blit(glm::uvec2 outputOffset, glm::uvec2 outputSize);
        void Blit(glm::uvec2 offset, glm::uvec2 size, glm::uvec2 outputOffset, glm::uvec2 outputSize);

        // Note: use to bind to a sampler2D
        void BindToTextureUnit(uint8_t textureUnitIndex);
        // Renders directly to a fullsreen quad using a default Shader.
        void Render();
        // Renders to a fullsreen quad with layout (vec2 position, vec2 UVs)
        void Render(Shader& shader, uint8_t textureUnitIndex);
        
        static void InitRender();
        static void ShutdownRender();

        inline uint32_t GetBuffer() const { return m_Buffer; }
        inline glm::uvec2 GetSize() const { return m_Size; }
        inline ColorFormat GetFormat() const { return m_Format; }
        inline ColorFilter GetFiltering() const { return m_Filter; }
    private:
        uint32_t m_Buffer;
        uint32_t m_Color;
        uint32_t m_DepthStencil = 0;

        ColorFormat m_Format;
        ColorFilter m_Filter;
        glm::uvec2 m_Size;
    };

    class MultisampleFramebuffer {
    public:
        enum class ColorFormat : uint8_t {
            HDR_F = 255,
            RGBA8 = 0,
            RG8,
            R8,
#if !defined(PLATFORM_EMSCRIPTEN)
            RGBA16,
            RG16,
            R16,
            RGBA16F,
            RG16F,
            R16F,
            RGBA32F,
            RG32F,
            R32F,
#endif
        };
        MultisampleFramebuffer(uint32_t width, uint32_t height, uint8_t sampleCount, MultisampleFramebuffer::ColorFormat format = MultisampleFramebuffer::ColorFormat::RGBA8, Framebuffer::ColorFilter filtering = Framebuffer::ColorFilter::Nearest, bool hasDepthStencil = true);
        ~MultisampleFramebuffer();

        void Resize(uint32_t width, uint32_t height, uint8_t sampleCount);
        
        void Activate();
        void Deactivate();

        void Blit(glm::uvec2 outputOffset, glm::uvec2 outputSize);
        void Blit(glm::uvec2 offset, glm::uvec2 size, glm::uvec2 outputOffset, glm::uvec2 outputSize);

        // Note: use to bind to a sampler2D, binds resolved (non MSAA) texture
        void BindToTextureUnit(uint8_t textureUnitIndex);
        // Renders directly to a fullsreen quad using a default Shader.
        void Render();
        // Renders to a fullsreen quad with layout (vec2 position, vec2 UVs)
        void Render(Shader& shader, uint8_t textureUnitIndex);

        inline static void InitRender() { Framebuffer::InitRender(); }
        inline static void ShutdownRender() { Framebuffer::ShutdownRender(); }

        inline uint32_t GetBuffer() const { return m_Buffer; }
        inline glm::uvec2 GetSize() const { return m_Resolved.GetSize(); }
        inline MultisampleFramebuffer::ColorFormat GetFormat() const { return static_cast<MultisampleFramebuffer::ColorFormat>(m_Resolved.GetFormat()); }
        inline Framebuffer::ColorFilter GetFiltering() const { return m_Resolved.GetFiltering(); }
    private:
        uint32_t m_Buffer;
        uint32_t m_Color;
        uint32_t m_DepthStencil = 0;

        Framebuffer m_Resolved;
    };
}
