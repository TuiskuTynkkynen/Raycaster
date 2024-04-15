#pragma once

#include "glm/glm.hpp"

namespace Core {
    struct Sprite {
        glm::vec3 Posistion;
        glm::vec3 Scale;

        glm::vec2 TexPosition;
        glm::vec2 TexScale;
        float TexRotation;
        uint32_t Atlasindex;

        glm::vec3 Colour;

        Sprite() {
            TexRotation = 0.0f;
            Atlasindex = 0;
        }
    };
    
    struct Tile {
        glm::vec3 Posistion;
        glm::vec3 Scale;

        bool IsTriangle;
        float Rotation;

        glm::vec3 Colour;

        Tile() {
            IsTriangle = false;
            Rotation = 0.0f;
        }
    };

    struct Line {
        glm::vec3 Posistion;
        glm::vec3 Scale;
    };

    struct Ray {
        glm::vec2 Position;
        float Scale;

        glm::vec2 TexPosition;
        float TexRotation;
        uint32_t Atlasindex;
        
        float Brightness;

        Ray() {
            Scale = 0.0f;
            TexRotation = 0.0f;
            Atlasindex = 0;
            Brightness = 0.0f;
        }
    };

    struct Player {
        glm::vec3 Position;
        glm::vec3 Scale;
        float Rotation;
    };
}