#include "Algorithms.h"

#include "Core/Debug/Debug.h"

#include <queue>

glm::u32vec2 Algorithms::AStar(glm::i32vec2 start, glm::i32vec2 end, std::vector<bool> map, uint32_t width, uint32_t height) {
    static const glm::i32vec2 directions[] = {
        glm::i32vec2(1,0),
        glm::i32vec2(-1,0),
        glm::i32vec2(0,1),
        glm::i32vec2(0,-1),
        glm::i32vec2(-1,-1),
        glm::i32vec2(-1,1),
        glm::i32vec2(1,-1),
        glm::i32vec2(1,1),
    };

    //Player on same tile as other enemy -> Pathfind to adjacent tile
    if (map[end.y * width + end.x]) {
        glm::i32vec2 test = end;
        for(uint32_t i = 0; i < 8 && map[test.y * width + test.x]; i++) {
            test = end + directions[i];
        }
        end = test;
    }

    std::vector<glm::i32vec2> previous;
    previous.assign(width * height, glm::i32vec2(-1, -1));

    std::vector<float> distance;
    distance.assign(width * height, UINT32_MAX);
    distance[start.y * width + start.x] = 0;

    typedef std::tuple<uint32_t, uint32_t, uint32_t> distanceYX;
    std::priority_queue<distanceYX, std::vector<distanceYX>, std::greater<distanceYX>> heap;
    heap.emplace(0, start.y, start.x);

    glm::i32vec2 current;
    while (!heap.empty()) {
        current.y = std::get<1>(heap.top());
        current.x = std::get<2>(heap.top());
        heap.pop();

        if (current == end) {
            break;
        }

        for (uint32_t i = 0; i < 4; i++) {
            glm::u32vec2 edge = current + directions[i];

            if (edge.x < 0 || edge.y < 0 || edge.x >= width || edge.y >= height || map[edge.y * width + edge.x]) {
                continue;
            }

            float tempDistance = distance[current.y * width + current.x] + 1;
            if (tempDistance < distance[edge.y * width + edge.x]) {
                distance[edge.y * width + edge.x] = tempDistance;
                previous[edge.y * width + edge.x] = current;
                int32_t distX = end.x - edge.x;
                int32_t distY = end.y - edge.y;
                heap.emplace(tempDistance + sqrt(abs(distX * distX + distY * distY)), edge.y, edge.x);
            }
        }
    }

    glm::u32vec2 result;
    while (current != start) {
        result = current;
        current = previous[current.y * width + current.x];
    }

    return result;
}

std::optional<glm::vec2> Algorithms::LineIntersection(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, glm::vec2 point4, bool isHalfLine) {
    glm::vec2 line1 = point2 - point1;
    glm::vec2 line2 = point4 - point3;

    float denominator = line1.x * line2.y - line2.x * line1.y;
    if (denominator == 0) {
        return std::nullopt; // Collinear
    }
    bool denomPositive = denominator > 0;

    glm::vec2 segment13 = point1 - point3;
    float check = line1.x * segment13.y - line1.y * segment13.x;
    if (!isHalfLine && (check < 0) == denomPositive || (check > denominator) == denomPositive) {
        return std::nullopt; //hits line2 but between point 3 and 4
    }

    float numerator = line2.x * segment13.y - line2.y * segment13.x;
    if ((numerator < 0) == denomPositive || (numerator > denominator) == denomPositive) {
        return std::nullopt; //hits line1 but between point 1 and 2
    }

    float t = numerator / denominator; //where point on line 1 -> 0 = p1 and 1 = p2 
    return glm::vec2(point1.x + (t * line1.x), point1.y + (t * line1.y));
}

glm::vec2 Algorithms::LineCollisions(glm::vec2 point, const std::vector<LineCollider>& lines, float thickness) {
    uint32_t lineCount = lines.size();
    glm::vec2 movement(0.0f);

    for (int i = 0; i < lineCount; i++) {
        float pointDX = point.x - lines[i].Position.x;
        float pointDY = point.y - lines[i].Position.y;
        float distance = std::max(lines[i].Vector.x * lines[i].Vector.x + lines[i].Vector.y * lines[i].Vector.y, 0.0001f);
        distance = (lines[i].Vector.x * pointDX + lines[i].Vector.y * pointDY) / distance;
      
        if (distance * lines[i].Length < -thickness * 0.5f || distance * lines[i].Length > lines[i].Length + thickness * 0.5f) {
            continue;
        }
        
        float dx = distance * lines[i].Vector.x + lines[i].Position.x;
        float dy = distance * lines[i].Vector.y + lines[i].Position.y;
        distance = sqrt((dx - point.x) * (dx - point.x) + (dy - point.y) * (dy - point.y));

        if (distance < thickness) {
            float crossProduct = lines[i].Vector.x * pointDY - lines[i].Vector.y * pointDX;

            if (crossProduct > 0) {
                movement.x -= point.x - (dx - lines[i].Normal.x * thickness);
                movement.y -= point.y - (dy - lines[i].Normal.y * thickness);
            }
            else {
                movement.x -= point.x - (dx + lines[i].Normal.x * thickness);
                movement.y -= point.y - (dy + lines[i].Normal.y * thickness);
            }
        }
    }

    return movement;
}

template<typename T>
std::vector<glm::vec<2, T, glm::highp>> MidpointCicleImpl(glm::vec<2, T, glm::highp> centre, T radius) {
    RC_ASSERT(radius > 0, "Radius of circle must be greater than 0");
    std::vector<glm::vec<2, T, glm::highp>> result;

    int32_t x = radius;
    int32_t y = 1;
    int32_t t = radius / 16 + y;

    result.emplace_back(centre.x + x, centre.y);
    result.emplace_back(centre.x - x, centre.y);
    result.emplace_back(centre.x, x + centre.y);
    result.emplace_back(centre.x, centre.y - x);

    while (x >= y) {
        result.emplace_back(centre.x + x, centre.y + y);
        result.emplace_back(centre.x - x, centre.y + y);
        result.emplace_back(centre.x - x, centre.y - y);
        result.emplace_back(centre.x + x, centre.y - y);

        if (x != y) {
            result.emplace_back(centre.x + y, centre.y + x);
            result.emplace_back(centre.x - y, centre.y - x);
            result.emplace_back(centre.x - y, centre.y + x);
            result.emplace_back(centre.x + y, centre.y - x);
        }

        t += ++y;
        if (t > x) {
            t -= x;
            x--;
        }
    }

    return result;
}
 
std::vector<glm::vec2> Algorithms::MidpointCicle(glm::vec2 centre, float radius) {
    return MidpointCicleImpl(centre, radius);
}
 
std::vector<glm::ivec2> Algorithms::MidpointCicle(glm::ivec2 centre, int32_t radius) {
    return MidpointCicleImpl(centre, radius);
}