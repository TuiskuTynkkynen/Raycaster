#pragma once

#include "Entities.h"

#include "glm/glm.hpp"

#include <optional>
#include <vector>
#include <span>

class Algorithms {
public:
    static glm::u32vec2 AStar(glm::i32vec2 start, glm::i32vec2 end, std::span<const bool> map, uint32_t width, uint32_t height);
    static std::optional<glm::vec2> LineIntersection(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, glm::vec2 point4, bool isHalfLine = false);
    static glm::vec2 LineCollisions(glm::vec2 point, std::span<const LineCollider> lines, float thickness);
    static std::vector<glm::vec2> MidpointCicle(glm::vec2 centre, uint32_t radius);
    static std::vector<glm::ivec2> MidpointCicle(glm::ivec2 centre, uint32_t radius);
};

