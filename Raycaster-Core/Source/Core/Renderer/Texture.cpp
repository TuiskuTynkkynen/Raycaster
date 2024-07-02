#include "Texture.h"
#include "Core/Debug/Debug.h"

#include <glad/glad.h>
#include <utils/stb_image.h>

#include <filesystem> 
#include <string>

namespace Core{
    Texture2D::Texture2D(GLint wrapS, GLint WrapT, GLint filterMin, GLint filterMax)
        : m_RendererID(0)
    {
        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMin);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMax);

        m_UsesMipMap = (filterMin == GL_NEAREST_MIPMAP_NEAREST || filterMin == GL_LINEAR_MIPMAP_NEAREST || filterMin == GL_NEAREST_MIPMAP_LINEAR || filterMin == GL_LINEAR_MIPMAP_LINEAR);
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

        switch (channelCount){
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