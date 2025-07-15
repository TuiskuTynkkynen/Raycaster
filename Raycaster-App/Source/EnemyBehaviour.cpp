#include <EnemyBehaviour.h>

static constexpr size_t s_DirectionCount = 8;
static constexpr std::array<glm::vec2, s_DirectionCount + 1> s_Directions = {
    glm::vec2(-1,-1),
    glm::vec2(0,-1),
    glm::vec2(1,-1),
    glm::vec2(-1,0),
    glm::vec2(1,0),
    glm::vec2(-1,1),
    glm::vec2(0,1),
    glm::vec2(1,1),
    glm::vec2(0,0),
};

static glm::vec2 Pathfind(const std::vector<float>& djikstraMap, glm::vec2 position, const Map& map) {
    glm::vec2 result(0.0f);

    glm::vec2 currentPosition = position;
    float min = djikstraMap[glm::floor(currentPosition.y) * map.GetWidth() + currentPosition.x];
    for (size_t i = 0; i < 5; i++) {
        size_t dir = s_DirectionCount;

        float minBias = -std::numeric_limits<float>::infinity();
        glm::vec2 fraction = glm::fract(currentPosition);
        for (size_t j = 0; j < s_DirectionCount; j++) {
            size_t x = currentPosition.x + s_Directions[j].x;
            size_t y = currentPosition.y + s_Directions[j].y;
            size_t index = y * map.GetWidth() + x;

            float val = djikstraMap[index];
            if (val < min) {
                min = val;
                dir = j;

                glm::vec2 distance{ (s_Directions[j].x > 0.0f) - s_Directions[j].x * fraction.x, (s_Directions[j].y > 0.0f) - s_Directions[j].y * fraction.y };
                float bias = glm::dot(distance, distance); 
                minBias = bias;
            } else if (val == min) { 
                // If multiple tile have the same value,
                // choose the tile closest to position
                glm::vec2 distance{ (s_Directions[j].x > 0.0f) - s_Directions[j].x * fraction.x, (s_Directions[j].y > 0.0f) - s_Directions[j].y * fraction.y };
                float bias = glm::dot(distance, distance); // Compare squared distance to save a sqrt
                
                if (bias < minBias) {
                    min = val;
                    dir = j;

                    minBias = bias ;
            }
        }
        }

        if (dir == s_DirectionCount) {
            break;
        }

        currentPosition += s_Directions[dir];
        if (i != 0 && !map.LineOfSight(position, currentPosition)) {
            break;
        }

        const float decay = 1.0f / (i * 0.75f + 1.0f);
        result += s_Directions[dir] * decay;
    }

    return result;
}

static glm::vec2 Collision(const Enemy& enemy, const Map& map, const std::vector<Enemy>& Enemies) {
    glm::vec2 result(0.0f);

    // TODO fix O(n^2) 
    for (size_t i = 0; i < Enemies.size(); i++) {
        //if (&enemy == &Enemies[i]) {
        //    continue;
        //}

        glm::vec2 dir = enemy.Position - Enemies[i].Position;

        // Check squared distance to save a sqrt
        if (glm::dot(dir, dir) > (0.45f * 0.45f)) {
            continue;
        }

        result += dir;
    }

    if (result.x != 0.0f || result.y != 0.0f) {
        result = glm::normalize(result);
    }

    size_t index = glm::floor(enemy.Position.y) * map.GetWidth() + enemy.Position.x;
    auto adjacent = map.GetNeighbours(index);

    if (adjacent.Bitboard) {
        glm::vec2 fraction = glm::fract(enemy.Position);

        for (size_t i = 0; i < s_DirectionCount; i++) {
            if (!adjacent[i]) {
                continue;
            }

            glm::vec2 dist(0.0f);
            dist.x += (s_Directions[i].x < 0) * 1.0f + s_Directions[i].x * fraction.x;
            dist.y += (s_Directions[i].y < 0) * 1.0f + s_Directions[i].y * fraction.y;

            // Check squared distance to save a sqrt
            if (glm::dot(dist, dist) > (0.4f * 0.4f)) {
                continue;
            }

            result += s_Directions[i];
        }
    }

    return result;
}

ActionStatus BasicAttack(Context& context, Enemy& enemy) {
    enemy.Tick += context.DeltaTime;
    enemy.AtlasIndex = GetAtlasIndex(enemy.Type) + 1;
    return ActionStatus::Done;
}

ActionStatus BasicPathfind(Context& context, Enemy& enemy) {
    enemy.Tick += context.DeltaTime * 2.0f;
    enemy.AtlasIndex = GetAtlasIndex(enemy.Type);
   
    glm::vec2 movementVector = Pathfind(context.AproachMap, enemy.Position, context.Map);
    if (movementVector.x != 0.0f || movementVector.y != 0.0f) {
    movementVector += glm::vec2{ 0.5f, 0.5f } - glm::fract(enemy.Position);
        movementVector = glm::normalize(movementVector);
    }

    if (movementVector.x != 0.0f || movementVector.y != 0.0f) {
        movementVector += glm::vec2{ 0.5f, 0.5f } - glm::fract(enemy.Position);
        movementVector = glm::normalize(movementVector);
    }
    
    glm::vec2 collision = Collision(enemy, context.Map, context.Enemies);
    if (collision.x != 0.0f || collision.y != 0.0f) {
        collision = glm::normalize(collision);
    }

    movementVector += collision;
    movementVector *= glm::min(context.DeltaTime.GetSeconds(), 1.0f) * GetSpeed(enemy.Type);
    
    auto oldPosition = enemy.Position;
    enemy.Position += movementVector;
    
    context.UpdateDjikstraMap |= glm::floor(oldPosition) != glm::floor(enemy.Position);

    return ActionStatus::Done;
}