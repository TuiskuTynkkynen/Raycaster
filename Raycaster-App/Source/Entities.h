#pragma once

#include "glm/glm.hpp"

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

struct Player {
    glm::vec3 Position{};
    glm::vec3 Scale{};
    float Rotation = 0.0f;
};

struct Floor {
    glm::vec2 Position{};
    float Length = 0.0f;

    glm::vec2 TexturePosition{};
    uint32_t AtlasIndex = 0;

    float Brightness = 0.0f;
};