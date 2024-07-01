#pragma once

#include "glm/glm.hpp"

namespace Core {
    struct Sprite {
        glm::vec3 Posistion{};
        glm::vec3 Scale{};

        glm::vec2 TexPosition{};
        glm::vec2 TexScale{};
        float TexRotation = 0.0f;
        uint32_t Atlasindex = 0;

        glm::vec3 Colour{};
    };
    
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
}