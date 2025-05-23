#pragma once

#include "glm/glm.hpp"

#include <optional>
#include <vector>

struct LineCollider {
    const glm::vec2 Position;
    const glm::vec2 Vector;
    const glm::vec2 Normal;
    const float Length;

    LineCollider(glm::vec2 point1, glm::vec2 point2)
        : Position(point1), Vector(point2 - point1), Length(glm::length(point2 - point1)),
        Normal(Vector.y / glm::length(point2 - point1), -Vector.x / glm::length(point2 - point1))
    {}
};

class Algorithms {
public:
    static bool LineOfSight(glm::vec2 start, glm::vec2 end, bool* map, uint32_t width, uint32_t height);
    static glm::u32vec2 AStar(glm::i32vec2 start, glm::i32vec2 end, bool* map, uint32_t width, uint32_t height);
    static std::optional<glm::vec2> LineIntersection(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, glm::vec2 point4, bool isHalfLine = false);
    static glm::vec2 LineCollisions(glm::vec2 point, const std::vector<LineCollider>& lines, float thickness);
};

