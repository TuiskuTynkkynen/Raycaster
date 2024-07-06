#include "Font.h"

#include "Core/Debug/Debug.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <filesystem>

namespace Core {
    Font::Font(bool interpolation) {
        uint32_t filter = interpolation ? GL_LINEAR : GL_NEAREST;
        m_TextureAtlas = std::make_unique<Texture2D>(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, filter, filter);
    }
    
    void Font::AddCharacterRange(uint32_t start, uint32_t end) {
        if (start > end) {
            uint32_t temp = start;
            start = end;
            end = temp;
        }
        
        m_CharacterRanges.emplace_back(start, end);
    }

    void Font::GenerateAtlas(const char* filePath, uint32_t height, uint32_t width) {
        uint32_t characterCount = 0;
        for (CharacterRange range : m_CharacterRanges) {
            characterCount += range.End - range.Start + 1; //Both start and end are inclusive
        }

        if (characterCount <= 0) {
            RC_WARN("NO CHARACTER RANGES HAVE BEEN ADDED TO FONT");
            return;
        }

        std::filesystem::path directoryPath = std::filesystem::current_path() / "Source" / "Fonts" / filePath;
        std::string fileString = directoryPath.string();

        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            RC_ERROR("FAILED TO INITIALIZE FREETYPE LIBRARY");
            return;
        }

        FT_Face face;
        if (FT_New_Face(ft, fileString.c_str(), 0, &face)) {
            FT_Done_FreeType(ft);
            RC_ERROR("FAILED TO LOAD FONT {}", filePath);
            return;
        }
        FT_Set_Pixel_Sizes(face, width, height);

        uint32_t textureDimensions = 1;
        uint32_t maxDimensions = (1 + (face->size->metrics.height >> 6)) * ceil(sqrtf(characterCount));
        //set textureDimensions to next largest power of two 
        while (textureDimensions < maxDimensions) {
            textureDimensions *= 2;
        }
        float pixelDimensions = 1.0f / textureDimensions;

        unsigned char* textureData = (unsigned char*)calloc(textureDimensions * textureDimensions, sizeof(unsigned char));
        uint32_t x = 0, y = 0;

        for (CharacterRange range : m_CharacterRanges) {
            for (uint32_t c = range.Start; c <= range.End; c++) {
                if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                    RC_WARN("FAILED TO LOAD GLYPH {}", c);
                    continue;
                }
                FT_Bitmap* bitmap = &face->glyph->bitmap;

                if (x + bitmap->width >= textureDimensions) {
                    x = 0;
                    y += ((face->size->metrics.height >> 6) + 1);
                }
                if (y + bitmap->rows >= textureDimensions) {
                    RC_WARN("COULD NOT FIT ALL CHARACTERS IN TEXTURE ATLAS");
                    break;
                }

                for (uint32_t row = 0; row < bitmap->rows; row++) {
                    for (uint32_t column = 0; column < bitmap->width; column++) {
                        uint32_t index = (y + row) * textureDimensions + x + column;
                        textureData[index] = bitmap->buffer[row * bitmap->pitch + column];
                    }
                }

                GlyphInfo glyph = {
                    glm::vec2(x * pixelDimensions, y * pixelDimensions),
                    glm::vec2(face->glyph->bitmap.width * pixelDimensions, face->glyph->bitmap.rows * pixelDimensions),
                    glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                    glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                    face->glyph->advance.x >> 6
                };
                m_Glyphs[c] = glyph;
                x += face->glyph->bitmap.width + 1;
            }
        }

        m_TextureAtlas->BindData(textureData, textureDimensions, textureDimensions, 1);
        free(textureData);

        //Store font height in space glyph
        if (!m_Glyphs.contains(' ')) {
            FT_Load_Char(face, ' ', FT_LOAD_RENDER);
            GlyphInfo space = {
                        glm::vec2(0.0f, 0.0f),
                        glm::vec2(0, (face->size->metrics.height >> 6)),
                        glm::ivec2(0, 0),
                        glm::ivec2(0, 0),
                        face->glyph->advance.x
            };
            m_Glyphs[' '] = space;
        }
        else {
            m_Glyphs[' '].Size.y = (face->size->metrics.height >> 6);
        }

        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }
}