#pragma once

#include "glm/glm.hpp"

class Algorithms
{
public:
    static bool LineOfSight(glm::vec2 start, glm::vec2 end, bool* map, uint32_t width, uint32_t height);
    static glm::u32vec2 AStar(glm::i32vec2 start, glm::i32vec2 end, bool* map, uint32_t width, uint32_t height);
};

