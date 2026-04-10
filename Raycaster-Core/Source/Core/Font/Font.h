#pragma once
#include "Core/Renderer/Texture.h"

#include <glm/glm.hpp>

#include <unordered_map>
#include <memory>
#include <vector>
#include <span>
#include <cstddef>

namespace Core {
    struct GlyphInfo {
        glm::vec2 TexPosition;
        glm::vec2 TexScale;
        glm::i32vec2 Size;
        glm::i32vec2 Bearing;
        uint32_t Advance;
    };

    class Font {
    public:
        struct CharacterRange {
            uint32_t Start;
            uint32_t End;
        };

        Font(bool interpolation = true);

        void AddCharacterRange(uint32_t start, uint32_t end);
        void GenerateAtlas(const char* filePath, uint32_t height, uint32_t width = 0);
        void GenerateAtlas(std::span<const std::byte> embededFont, uint32_t height, uint32_t width = 0);
        void GenerateSDFAtlas(const char* filePath, uint32_t height, uint32_t width = 0); //TODO gen on seperate thread
        void GenerateSDFAtlas(std::span<const std::byte> embededFont, uint32_t height, uint32_t width = 0);

        inline void ActivateAtlas(uint32_t unitIndex = 0) { m_TextureAtlas->Activate(unitIndex); }
        inline GlyphInfo GetGlyphInfo(uint32_t character) { return m_Glyphs[character]; }
    private:
        std::vector<CharacterRange> m_CharacterRanges;
        std::unordered_map<uint32_t, GlyphInfo> m_Glyphs;
        std::unique_ptr<Core::Texture2D> m_TextureAtlas;
    };
}

