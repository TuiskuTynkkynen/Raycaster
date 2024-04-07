#pragma once

#include "glm/glm.hpp"

namespace Core {
    struct Sprite {
        glm::vec3 Posistion;
        glm::vec3 Scale;

        glm::vec2 TexPosition;
        glm::vec2 TexScale;
        float TexRotation;
        float Atlasindex;

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
        glm::vec2 Position;
        float Scale;

        glm::vec2 TexPosition;
        float TexRotation;
        float Atlasindex;
        
        float Brightness;
    };

    struct Player {
        glm::vec3 Position;
        glm::vec3 Scale;
        float Rotation;
    };
}