#include "Core/Font/Font.h"

#include "Core/Font/Outline.h"
#include "Core/Debug/Debug.h"
#include "Platform.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#include <filesystem>

namespace Core {
    Font::Font(bool interpolation) {
        Texture2D::Filter filter = interpolation ? Texture2D::Filter::Linear : Texture2D::Filter::Nearest;
        m_TextureAtlas = std::make_unique<Texture2D>(Texture2D::WrapMode::ClampToEdge, Texture2D::WrapMode::ClampToEdge, filter, filter);
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

        std::string fileString = ApplicationDirectory().append(filePath).string();

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
        uint32_t maxDimensions = (1 + (face->size->metrics.height >> 6)) * static_cast<uint32_t>(glm::ceil(glm::sqrt(characterCount)));
        //set textureDimensions to next largest power of two 
        while (textureDimensions < maxDimensions) {
            textureDimensions *= 2;
        }
        float pixelDimensions = 1.0f / textureDimensions;

        std::vector<unsigned char>textureData(textureDimensions * textureDimensions);
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
                    static_cast<uint32_t>(face->glyph->advance.x >> 6)
                };
                m_Glyphs[c] = glyph;
                x += face->glyph->bitmap.width + 1;
            }
        }

        m_TextureAtlas->BindData(textureData.data(), textureDimensions, textureDimensions, 1);

        //Store font height in space glyph
        if (!m_Glyphs.contains(' ')) {
            FT_Load_Char(face, ' ', FT_LOAD_RENDER);
            GlyphInfo space = {
                        glm::vec2(0.0f, 0.0f),
                        glm::vec2(0, (face->size->metrics.height >> 6)),
                        glm::ivec2(0, 0),
                        glm::ivec2(0, 0),
                        static_cast<uint32_t>(face->glyph->advance.x)
            };
            m_Glyphs[' '] = space;
        }
        else {
            m_Glyphs[' '].Size.y = (face->size->metrics.height >> 6);
        }

        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }

    void Font::GenerateSDFAtlas(const char* filePath, uint32_t height, uint32_t width) {
        uint32_t characterCount = 0;
        for (CharacterRange range : m_CharacterRanges) {
            characterCount += range.End - range.Start + 1; //Both start and end are inclusive
        }

        if (characterCount <= 0) {
            RC_WARN("NO CHARACTER RANGES HAVE BEEN ADDED TO FONT");
            return;
        }

        RC_INFO("Generating SDF font with {} characters", characterCount);

        std::string fileString = ApplicationDirectory().append(filePath).string();

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

        const uint32_t scaleFactor = 1;
        FT_Set_Pixel_Sizes(face, width * scaleFactor, height * scaleFactor);

        uint32_t textureDimensions = 1;
        uint32_t maxDimensions = (1 + ((2 + face->size->metrics.height) >> 6)) * static_cast<uint32_t>(glm::ceil(glm::sqrt(characterCount)));
        //set textureDimensions to next largest power of two 
        while (textureDimensions < maxDimensions) {
            textureDimensions *= 2;
        }
        float pixelDimensions = 1.0f / textureDimensions;

        Core::SDF::Outline glyph;
        std::vector<unsigned char>textureData(textureDimensions * textureDimensions);
        uint32_t x = 0, y = 0;

        for (CharacterRange range : m_CharacterRanges) {
            for (uint32_t c = range.Start; c <= range.End; c++) {
                if (FT_Load_Char(face, c, FT_LOAD_NO_HINTING)) {
                    RC_WARN("FAILED TO LOAD GLYPH {}", c);
                    continue;
                }

                FT_Outline outline = face->glyph->outline;
                FT_BBox box;
                FT_Outline_Get_CBox(&outline, &box);

                if ((box.xMax - box.xMin) * (box.yMax - box.yMin) == 0) {
                    m_Glyphs[c].Advance = face->glyph->advance.x >> 6;
                    continue;
                }

                const uint32_t width = ((box.xMax - box.xMin) >> 6) + 3;
                const uint32_t rows = ((box.yMax - box.yMin) >> 6) + 3;

                if (x + width >= textureDimensions) {
                    x = 0;
                    y += ((face->size->metrics.height >> 6) + 1);
                }
                if (y + rows >= textureDimensions) {
                    RC_WARN("COULD NOT FIT ALL CHARACTERS IN TEXTURE ATLAS");
                    break;
                }

                glyph.Init(box, &outline);
                const float spread = glm::sqrt(static_cast<float>(width * width + rows * rows));

                for (uint32_t row = 0; row < rows; row++) {
                    for (uint32_t column = 0; column < width; column++) {
                        const glm::vec2 point(column - 0.5f, row - 0.5f);

                        float len = std::max(glyph.DistanceTo(point), 0.1f);
                        len = std::min(len / spread, 0.5f);
                        len = glyph.ContainsPoint(point) ? 0.5f + len : 0.5f - len;

                        const uint32_t index = (y + row) * textureDimensions + x + column;
                        if (index < textureDimensions * textureDimensions) {
                            textureData[index] = static_cast<unsigned char>(len * std::numeric_limits<unsigned char>::max());
                        }

                    }
                }

                const GlyphInfo glyphInfo = {
                                    glm::vec2(x * pixelDimensions, y * pixelDimensions),
                                    glm::vec2(width * pixelDimensions, rows * pixelDimensions),
                                    glm::ivec2(width, rows),
                                    glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                                    static_cast<uint32_t>(face->glyph->advance.x >> 6)
                };
                m_Glyphs[c] = glyphInfo;
                x += width + 1;
            }
        }

        m_TextureAtlas->BindData(textureData.data(), textureDimensions, textureDimensions, 1);

        //Store font height in space glyph
        if (!m_Glyphs.contains(' ')) {
            FT_Load_Char(face, ' ', FT_LOAD_NO_HINTING);
            GlyphInfo space = {
                        glm::vec2(0.0f, 0.0f),
                        glm::vec2(0, (face->size->metrics.height >> 6)),
                        glm::ivec2(0, 0),
                        glm::ivec2(0, 0),
                        static_cast<uint32_t>(face->glyph->advance.x)
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
