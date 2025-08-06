#pragma once

#include "TextureIndices.h"
#include "Core/Base/Timestep.h"

#include "glm/glm.hpp"

#include <array>
#include <variant>

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

struct MeleeWeaponData {
    float AttackLength = 0.0f;
    float AttackThickness = 0.0f;
    float Damage = 0.0f;
    float AttackTiming = 0.0f;
};

struct Item {
    float Scale = 0.0f;
    uint32_t Count = 0;
    
    AtlasAnimation UseAnimation{};
    float UseDuration = 0.0f;
    std::variant<MeleeWeaponData> AdditionalData;
};

struct Player {
    glm::vec3 Position{};
    glm::vec3 Scale{};
    float Rotation = 0.0f;
    static constexpr float MaxHealth = 10.0f;
    float Health = MaxHealth;

    Core::Timestep AnimationProgress = -std::numeric_limits<float>::infinity();
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