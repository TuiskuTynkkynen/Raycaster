#pragma once

#include "glm/glm.hpp"

#include <optional>

class Algorithms {
public:
    static bool LineOfSight(glm::vec2 start, glm::vec2 end, bool* map, uint32_t width, uint32_t height);
    static glm::u32vec2 AStar(glm::i32vec2 start, glm::i32vec2 end, bool* map, uint32_t width, uint32_t height);
    static std::optional<glm::vec2> LineIntersection(glm::vec2& point1, glm::vec2& point2, glm::vec2& point3, glm::vec2& point4, bool isHalfLine = false);
};

