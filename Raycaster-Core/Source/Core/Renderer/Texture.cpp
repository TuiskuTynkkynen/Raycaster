#include "Texture.h"
#include "Core/Debug/Debug.h"

#include <glad/glad.h>
#include <utils/stb_image.h>

#include <filesystem> 
#include <string>

namespace Core {
    static constexpr GLint ToGLint(Texture2D::WrapMode mode) {
        switch (mode) {
        case Texture2D::WrapMode::Repeat:
            return GL_REPEAT;
        case Texture2D::WrapMode::MirroredRepeat:
            return GL_MIRRORED_REPEAT;
        case Texture2D::WrapMode::ClampToEdge:
            return GL_CLAMP_TO_EDGE;
        }

        RC_ASSERT("This should not be reached");
        return 0;
    }

    static constexpr GLint ToGLint(Texture2D::Filter filter) {
        switch (filter) {
        case Texture2D::Filter::Nearest:
            return GL_NEAREST;
        case Texture2D::Filter::Linear:
            return GL_LINEAR;
        }

        RC_ASSERT("This should not be reached");
        return 0;
    }

    static constexpr GLint ToGLint(Texture2D::MipmapFilter mipmap, Texture2D::Filter texture) {
        switch (mipmap) {
        case Texture2D::MipmapFilter::Disabled:
            return ToGLint(texture);
        case Texture2D::MipmapFilter::Nearest:
            return (texture == Texture2D::Filter::Linear) ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST;
        case Texture2D::MipmapFilter::Linear:
            return (texture == Texture2D::Filter::Linear) ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR;
        }

        RC_ASSERT("This should not be reached");
        return 0;
    }

    Texture2D::Texture2D(WrapMode S, WrapMode T, Filter minification, Filter magnification, MipmapFilter mipmap)
        : m_RendererID(0) {
        GL_TEXTURE_WRAP_S;
        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ToGLint(S));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ToGLint(T));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ToGLint(mipmap, minification));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ToGLint(magnification));

        m_UsesMipMap = (mipmap != MipmapFilter::Disabled);
    }

    Texture2D::~Texture2D() {
        glDeleteTextures(1, &m_RendererID);
    }

    void Texture2D::BindImage(const char* fileName) {
        std::filesystem::path directoryPath = std::filesystem::current_path() / "Source" / "Textures";
        std::string fileString = directoryPath.append(fileName).string();
        const char* filePath = fileString.c_str();

        stbi_set_flip_vertically_on_load(true);

        int textureWidth, textureHeight, textureChannelCount;
        unsigned char* data = stbi_load(filePath, &textureWidth, &textureHeight, &textureChannelCount, 0);

        if (data) {
            GLenum colourSpace;

            switch (textureChannelCount)
            {
            case 1:
                colourSpace = GL_RED;
                break;
            case 3:
                colourSpace = GL_RGB;
                break;
            case 4:
                colourSpace = GL_RGBA;
                break;
            default:
                colourSpace = GL_RGB;
                break;
            }

            glBindTexture(GL_TEXTURE_2D, m_RendererID);
            glTexImage2D(GL_TEXTURE_2D, 0, colourSpace, textureWidth, textureHeight, 0, colourSpace, GL_UNSIGNED_BYTE, data);
            if (m_UsesMipMap) {
                glGenerateMipmap(GL_TEXTURE_2D);
            }
        }
        else {
            RC_ERROR("FAILED TO LOAD TEXTURE {}", fileName);
        }
        stbi_image_free(data);
    }

    void Texture2D::BindData(const unsigned char* data, uint32_t height, uint32_t width, uint32_t channelCount) {
        GLenum colourSpace;

        switch (channelCount) {
        case 1:
            colourSpace = GL_RED;
            break;
        case 3:
            colourSpace = GL_RGB;
            break;
        case 4:
            colourSpace = GL_RGBA;
            break;
        default:
            colourSpace = GL_RGB;
            break;
        }

        glBindTexture(GL_TEXTURE_2D, m_RendererID);
        glTexImage2D(GL_TEXTURE_2D, 0, colourSpace, width, height, 0, colourSpace, GL_UNSIGNED_BYTE, data);
        if (m_UsesMipMap) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
    }

    void Texture2D::Activate(uint32_t unitIndex) {
        if (unitIndex > 15) { unitIndex = 0; }
        glActiveTexture(GL_TEXTURE0 + unitIndex);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
    }
}