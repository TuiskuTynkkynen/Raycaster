#pragma once

#include "Core/Base/Timestep.h"

#include "glm/glm.hpp"

#include <array>

inline constexpr uint32_t ATLASWIDTH = 11;
inline constexpr uint32_t ATLASHEIGHT = 2;

struct Tile {
    glm::vec3 Posistion{};
    glm::vec3 Scale{};

    bool IsTriangle = false;
    float Rotation = 0.0f;
    
    glm::vec3 Colour{};
};

struct Line {
    glm::vec3 Posistion{};
    glm::vec3 Scale{};
};

struct Ray {
    glm::vec2 Position{};
    float Scale = 0.0f;

    glm::vec2 TexPosition{};
    float TexRotation = 0.0f;
    uint32_t Atlasindex = 0;

    float Brightness = 0.0f;
};

struct AtlasAnimation {
    uint16_t StartAtlasIndex = 0;
    uint16_t FrameCount = 0;
    
    constexpr uint32_t GetFrame(Core::Timestep progress) const {
        // Subtract epsilon, since animation should be an exclusive range [StartAtlasIndex, StartAtlasIndex + Framecount)
        return StartAtlasIndex + static_cast<uint32_t>((FrameCount - std::numeric_limits<float>::epsilon()) * progress);
    }
};

struct Item {
    float Scale = 0.0f;
    uint32_t AtlasIndex = 0;
    uint32_t Count = 0;
};

struct Player {
    glm::vec3 Position{};
    glm::vec3 Scale{};
    float Rotation = 0.0f;

    size_t HeldItem = 0;
    std::array<Item, 1> Inventory;
};

struct Floor {
    glm::vec2 Position{};
    float Length = 0.0f;

    glm::vec2 TexturePosition{};
    uint16_t BottomAtlasIndex = 0;
    uint16_t TopAtlasIndex = 0;

    float BrightnessStart = 0.0f;
    float BrightnessEnd = 0.0f;
};

struct Sprite {
    glm::vec3 Position{};
    glm::vec3 WorldPosition{};
    glm::vec3 Scale{};

    uint32_t AtlasIndex = 0;
    bool FlipTexture = false;
};