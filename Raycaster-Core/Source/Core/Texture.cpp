#include "Texture.h"

#include <glad/glad.h>
#include <stb_image.h>

#include <filesystem> 
#include <iostream>
#include <string>
namespace Core{
	Texture2D::Texture2D(GLint wrapS, GLint WrapT, GLint filterMin, GLint filterMax) {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMin);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMax);

        usesMipMap = (filterMin == GL_NEAREST_MIPMAP_NEAREST, filterMin == GL_LINEAR_MIPMAP_NEAREST, filterMin == GL_NEAREST_MIPMAP_LINEAR, filterMin == GL_LINEAR_MIPMAP_LINEAR);
	}

    void Texture2D::BindImage(const char* fileName, GLenum colourSpace) {
        std::filesystem::path directoryPath = std::filesystem::current_path().parent_path() / "Raycaster-Core" / "Source" / "Textures";
        std::string fileString = directoryPath.append(fileName).string();
        const char* filePath = fileString.c_str();

        int textureWidth, textureHeight, textureChannelCount;
        unsigned char* data = stbi_load(filePath, &textureWidth, &textureHeight, &textureChannelCount, 0);

        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, colourSpace, GL_UNSIGNED_BYTE, data);
            if (usesMipMap) {
                glGenerateMipmap(GL_TEXTURE_2D);
            }
        }
        else {
            std::cout << "ERROR: FAILED TO LOAD TEXTURE" << filePath << std::endl;
        }
        stbi_image_free(data);
    }

    void Texture2D::Activate(uint32_t unitIndex) {
        if (unitIndex > 15) { unitIndex = 0; }
        glActiveTexture(GL_TEXTURE0 + unitIndex);
        glBindTexture(GL_TEXTURE_2D, ID);
    }
}