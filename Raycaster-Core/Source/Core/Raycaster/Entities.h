#pragma once

#include "glm/glm.hpp"

namespace Core {
    struct Sprite {
        glm::vec3 Posistion;
        glm::vec3 Scale;

        glm::vec2 TexPosition;
        glm::vec2 TexScale;
        float TexRotation;

        glm::vec3 Colour;
    };
    
    struct FlatQuad {
        glm::vec3 Posistion;
        glm::vec3 Scale;

        glm::vec3 Colour;
    };

    struct Line {
        glm::vec3 Posistion;
        glm::vec3 Scale;
    };

    struct Ray {
        float rayPosX;
        float rayScaleY;
        float texPosX;
        float brightness;
    };

    struct Player {
        glm::vec3 Position;
        glm::vec3 Scale;
        float Rotation;
    };
}