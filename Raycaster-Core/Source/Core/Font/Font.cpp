#include "Core/Font/Font.h"

#include "Core/Font/Outline.h"
#include "Core/Debug/Debug.h"
#include "Platform.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#include <filesystem>

namespace Core {
    static std::optional<std::pair<FT_Library, FT_Face>> InitFreeType(const char* filePath, uint32_t height, uint32_t width) {
        std::string fileString = ApplicationDirectory().append(filePath).string();

        FT_Library library;
        if (FT_Init_FreeType(&library)) {
            RC_ERROR("Can not generate Font atlas. Failed to initialize FreeType library.");
            return std::nullopt;
        }

        FT_Face face;
        if (FT_New_Face(library, fileString.c_str(), 0, &face)) {
            RC_ERROR("Can not generate Font atlas. Failed to load font from file, {}", filePath);
            FT_Done_FreeType(library);
            return std::nullopt;
        }

        FT_Set_Pixel_Sizes(face, width, height);
        return std::make_pair(library, face);
    }
    
    static std::optional<std::pair<FT_Library, FT_Face>> InitFreeType(std::span<const std::byte> embededFont, uint32_t height, uint32_t width) {
        FT_Library library;
        if (FT_Init_FreeType(&library)) {
            RC_ERROR("Can not generate Font atlas. Failed to initialize FreeType library.");
            return std::nullopt;
        }

        FT_Face face;

        const FT_Byte* buffer = reinterpret_cast<const FT_Byte*>(embededFont.data());
        if (FT_New_Memory_Face(library, buffer, static_cast<FT_Long>(embededFont.size()), 0, &face)) {
            RC_ERROR("Can not generate Font atlas. Failed to load embeded font");
            FT_Done_FreeType(library);
            return std::nullopt;
        }

        FT_Set_Pixel_Sizes(face, width, height);
        return std::make_pair(library, face);
    }

    static void GenerateAtlasInternal(FT_Face& face, std::span<const Font::CharacterRange> characterRanges, std::unordered_map<uint32_t, GlyphInfo>& glyphs, Texture2D& textureAtlas) {
        uint32_t characterCount = 0;
        for (Font::CharacterRange range : characterRanges) {
            characterCount += range.End - range.Start + 1; //Both start and end are inclusive
        }

        if (characterCount <= 0) {
            RC_WARN("Can not generate Font atlas. No character ranges have been added to Font.");
            return;
        }

        uint32_t textureDimensions = 1;
        uint32_t maxDimensions = (1 + (face->size->metrics.height >> 6)) * static_cast<uint32_t>(glm::ceil(glm::sqrt(characterCount)));
        //set textureDimensions to next largest power of two 
        while (textureDimensions < maxDimensions) {
            textureDimensions *= 2;
        }
        float pixelDimensions = 1.0f / textureDimensions;

        std::vector<unsigned char>textureData(textureDimensions * textureDimensions);
        uint32_t x = 0, y = 0;
        for (Font::CharacterRange range : characterRanges) {
            for (uint32_t c = range.Start; c <= range.End; c++) {
                if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                    RC_WARN("Failed to load glyph, {}, while generating Font atlas", c);
                    continue;
                }
                FT_Bitmap* bitmap = &face->glyph->bitmap;

                if (x + bitmap->width >= textureDimensions) {
                    x = 0;
                    y += ((face->size->metrics.height >> 6) + 1);
                }
                if (y + bitmap->rows >= textureDimensions) {
                    RC_WARN("Could not fit all characters to Font texture atlas.");
                    break;
                }

                for (uint32_t row = 0; row < bitmap->rows; row++) {
                    for (uint32_t column = 0; column < bitmap->width; column++) {
                        uint32_t index = (y + row) * textureDimensions + x + column;
                        textureData[index] = bitmap->buffer[row * bitmap->pitch + column];
                    }
                }

                glyphs[c] = {
                    glm::vec2(x * pixelDimensions, y * pixelDimensions),
                    glm::vec2(face->glyph->bitmap.width * pixelDimensions, face->glyph->bitmap.rows * pixelDimensions),
                    glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                    glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                    static_cast<uint32_t>(face->glyph->advance.x >> 6)
                };
                x += face->glyph->bitmap.width + 1;
            }
        }

        textureAtlas.BindData(textureData.data(), textureDimensions, textureDimensions, 1);

        //Store font height in space glyph
        if (!glyphs.contains(' ')) {
            FT_Load_Char(face, ' ', FT_LOAD_RENDER);
            glyphs[' '] = {
                glm::vec2(0.0f, 0.0f),
                glm::vec2(0, (face->size->metrics.height >> 6)),
                glm::ivec2(0, 0),
                glm::ivec2(0, 0),
                static_cast<uint32_t>(face->glyph->advance.x)
            };
        }

        glyphs[' '].Size.y = (face->size->metrics.height >> 6);
    }

    static void GenerateSDFAtlasInternal(FT_Face& face, std::span<const Font::CharacterRange> characterRanges, std::unordered_map<uint32_t, GlyphInfo>& glyphs, Texture2D& textureAtlas) {
        uint32_t characterCount = 0;
        for (Font::CharacterRange range : characterRanges) {
            characterCount += range.End - range.Start + 1; //Both start and end are inclusive
        }

        if (characterCount <= 0) {
            RC_WARN("Can not generate SDF Font atlas. No character ranges have been added to Font.");
            return;
        }

        RC_INFO("Generating SDF Font with {} characters", characterCount);

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
        for (Font::CharacterRange range : characterRanges) {
            for (uint32_t c = range.Start; c <= range.End; c++) {
                if (FT_Load_Char(face, c, FT_LOAD_NO_HINTING)) {
                    RC_WARN("Failed to load glyph, {}, while generating SDF Font atlas", c);
                    continue;
                }

                FT_Outline outline = face->glyph->outline;
                FT_BBox box;
                FT_Outline_Get_CBox(&outline, &box);

                if ((box.xMax - box.xMin) * (box.yMax - box.yMin) == 0) {
                    glyphs[c].Advance = face->glyph->advance.x >> 6;
                    continue;
                }

                const uint32_t width = ((box.xMax - box.xMin) >> 6) + 3;
                const uint32_t rows = ((box.yMax - box.yMin) >> 6) + 3;

                if (x + width >= textureDimensions) {
                    x = 0;
                    y += ((face->size->metrics.height >> 6) + 1);
                }
                if (y + rows >= textureDimensions) {
                    RC_WARN("Could not fit all characters to SDF Font texture atlas.");
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

                glyphs[c] = {
                    glm::vec2(x * pixelDimensions, y * pixelDimensions),
                    glm::vec2(width * pixelDimensions, rows * pixelDimensions),
                    glm::ivec2(width, rows),
                    glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                    static_cast<uint32_t>(face->glyph->advance.x >> 6)
                };
                x += width + 1;
            }
        }

        textureAtlas.BindData(textureData.data(), textureDimensions, textureDimensions, 1);

        //Store font height in space glyph
        if (!glyphs.contains(' ')) {
            FT_Load_Char(face, ' ', FT_LOAD_NO_HINTING);
            glyphs[' '] = {
                glm::vec2(0.0f, 0.0f),
                glm::vec2(0, (face->size->metrics.height >> 6)),
                glm::ivec2(0, 0),
                glm::ivec2(0, 0),
                static_cast<uint32_t>(face->glyph->advance.x)
            };
        }

        glyphs[' '].Size.y = (face->size->metrics.height >> 6);
    }

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
        RC_ASSERT(m_TextureAtlas.get(), "Font must always have a valid texture.");
        
        auto freetype = InitFreeType(filePath, height, width);
        if (!freetype) {
            return;
        }

        auto& [library, face] = freetype.value();
        GenerateAtlasInternal(face, m_CharacterRanges, m_Glyphs, *m_TextureAtlas.get());
        
        FT_Done_Face(face);
        FT_Done_FreeType(library);
    }

    void Font::GenerateAtlas(std::span<const std::byte> embededFont, uint32_t height, uint32_t width) {
        RC_ASSERT(m_TextureAtlas.get(), "Font must always have a valid texture.");
        
        auto freetype = InitFreeType(embededFont, height, width);
        if (!freetype) {
            return;
        }

        auto& [library, face] = freetype.value();
        GenerateAtlasInternal(face, m_CharacterRanges, m_Glyphs, *m_TextureAtlas.get());
        
        FT_Done_Face(face);
        FT_Done_FreeType(library);
    }

    void Font::GenerateSDFAtlas(const char* filePath, uint32_t height, uint32_t width) {
        RC_ASSERT(m_TextureAtlas.get(), "Font must always have a valid texture.");

        auto freetype = InitFreeType(filePath, height, width);
        if (!freetype) {
            return;
        }

        auto& [library, face] = freetype.value();
        GenerateSDFAtlasInternal(face, m_CharacterRanges, m_Glyphs, *m_TextureAtlas.get());

        FT_Done_Face(face);
        FT_Done_FreeType(library);
    }

    void Font::GenerateSDFAtlas(std::span<const std::byte> embededFont, uint32_t height, uint32_t width) {
        RC_ASSERT(m_TextureAtlas.get(), "Font must always have a valid texture.");

        auto freetype = InitFreeType(embededFont, height, width);
        if (!freetype) {
            return;
        }

        auto& [library, face] = freetype.value();
        GenerateSDFAtlasInternal(face, m_CharacterRanges, m_Glyphs, *m_TextureAtlas.get());

        FT_Done_Face(face);
        FT_Done_FreeType(library);
    }
}
