#pragma once

#include  <cstdint>

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

        void BindImage(const char* fileName);
        void BindData(const unsigned char* data, uint32_t height, uint32_t width, uint32_t channelCount);
        void Activate(uint32_t unitIndex);
    private:
        uint32_t m_RendererID;
        bool m_UsesMipMap;
    };
}

