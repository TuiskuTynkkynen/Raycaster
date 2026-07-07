#pragma once

#include <cstdint>
#include <span>
#include <cstddef>
#include <utility>

namespace Core {
    class Texture2D {
    public:
        enum class WrapMode {
            Repeat = 0,
            MirroredRepeat,
            ClampToEdge,
        };

        enum class Filter {
            Nearest = 0,
            Linear,
        };

        enum class MipmapFilter {
            Disabled = 0, // Disabled mipmaps
            Nearest,
            Linear,
        };

        Texture2D(WrapMode S, WrapMode T, Filter minification, Filter magnification, MipmapFilter mipmap = MipmapFilter::Disabled);
        ~Texture2D();

        Texture2D(const Texture2D&) = delete;
        Texture2D(Texture2D&& other) noexcept
            : m_RendererID(std::exchange(other.m_RendererID, 0)), m_UsesMipMap(other.m_UsesMipMap) {};
        Texture2D& operator=(const Texture2D&) = delete;
        Texture2D& operator=(Texture2D&& other) noexcept {
            std::swap(m_RendererID, other.m_RendererID); std::swap(m_UsesMipMap, other.m_UsesMipMap);
            return *this;
        };

        void BindImage(const char* fileName);
        void BindImage(std::span<const std::byte> embededImage);
        void BindData(const unsigned char* data, uint32_t height, uint32_t width, uint32_t channelCount);
        void Activate(uint32_t unitIndex);
    private:
        uint32_t m_RendererID;
        bool m_UsesMipMap;
    };
}

