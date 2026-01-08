#include "EnemyBehaviour.h"

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

bool LineOfSight(const Context& context, glm::vec2 start, glm::vec2 end) {
    // NOTE does not check start tile 

    // Brute forcing would be faster for small enemy counts,
    // but overengineering is way more fun

    if (!context.Map.LineOfSight(start, end)) {
        return false;
    }

    const std::array<const LineCollider, 1> collider = { LineCollider(start, end) };
    auto checkCollision = [context, collider](uint8_t& bitfield, uint16_t mapX, uint16_t mapY, uint16_t width) -> bool {
        for (uint16_t y = 0; y < 3; y++) {
            for (uint16_t x = 0; x < 3; x++) {
                uint16_t bitIndex = y * 3 + x;
                // Move values to 0-8 range (x == 0 && y == 0 does not need to be checked -> maskBit = 0)
                bitIndex = (bitIndex == 4) ? 9 : bitIndex - (bitIndex > 4);
                uint8_t maskBit = 1 << bitIndex;

                if (!maskBit || (bitfield & maskBit) || mapX + x >= width) {
                    continue;
                }

                auto span = context.SpatialPartition.Get({ mapX + x - 1, mapY + y - 1 });
                for (uint16_t index : span) {
                    auto hit = Algorithms::LineCollisions(context.Enemies[index].Position, collider, context.Enemies[index].Scale().x * 0.5f) != glm::vec2(0.0f);
                    if (hit) {
                        return true;
                    }
                }

                bitfield |= maskBit;
            }
        }

        return false;
        };

    glm::vec2 rayDirection = end - start;
    glm::vec2 deltaDistance = glm::abs(1.0f / glm::normalize(rayDirection));

    uint16_t mapX = static_cast<uint16_t>(start.x);
    uint16_t mapY = static_cast<uint16_t>(start.y);

    int32_t stepX = (rayDirection.x > 0.0f) ? 1 : -1;
    int32_t stepY = (rayDirection.y > 0.0f) ? 1 : -1;

    glm::vec2 sideDistance = deltaDistance;
    sideDistance.x *= (rayDirection.x < 0.0f) ? (start.x - mapX) : (mapX + 1.0f - start.x);
    sideDistance.y *= (rayDirection.y < 0.0f) ? (start.y - mapY) : (mapY + 1.0f - start.y);

    uint8_t bitfield = 0;
    while (mapX != static_cast<uint32_t>(end.x) || mapY != static_cast<uint32_t>(end.y)) {
        if (checkCollision(bitfield, mapX, mapY, context.Map.GetWidth())) {
            return false;
        }

        if (sideDistance.x < sideDistance.y) {
            sideDistance.x += deltaDistance.x;
            mapX += stepX;

            bitfield &= (stepX == 1) ? 0b01101011 : 0b11010110;
        }
        else {
            sideDistance.y += deltaDistance.y;
            mapY += stepY;

            (stepY == 1) ? (bitfield >>= 3) : (bitfield <<= 3);
        }

        if (mapY >= context.Map.GetHeight() || mapX >= context.Map.GetWidth()) {
            return false;
        }
    }

    if (checkCollision(bitfield, mapX, mapY, context.Map.GetWidth())) {
        return false;
    }

    return true;
}

static glm::vec2 Pathfind(const std::vector<float>& djikstraMap, glm::vec2 position, const Map& map) {
    glm::vec2 result(0.0f);
    
    glm::ivec2 flooredPosition = position;
    glm::vec2 fraction = glm::fract(position);

    glm::vec2 currentPosition = position;
    float min = djikstraMap[map.GetIndex(currentPosition)];
    for (size_t i = 0; i < 5; i++) {
        size_t dir = s_DirectionCount;

        float minBias = -std::numeric_limits<float>::infinity();
        for (size_t j = 0; j < s_DirectionCount; j++) {
            glm::ivec2 delta = currentPosition + s_Directions[j];
            size_t index = map.GetIndex(delta.x, delta.y);

            // Substract the cost current enemy adds to the dijkstra map
            // This prevents enemy "jiggling" on tile borders
            delta = glm::abs(delta - flooredPosition);
            int cost = 4 - delta.x - delta.y;
            
            float val = djikstraMap[index] - cost * (cost > 0);
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

    size_t index = map.GetIndex(enemy.Position);
    auto adjacent = map.GetNeighbours(index);

    if (adjacent.Bitboard) {
        glm::vec2 fraction = glm::fract(enemy.Position);

        for (size_t i = 0; i < s_DirectionCount; i++) {
            if (!adjacent[i] || map.IsPassable(index + static_cast<int32_t>(s_Directions[s_DirectionCount - 1 - i].y * map.GetWidth() + s_Directions[s_DirectionCount - 1 - i].x))) {
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
    Core::Timestep relativeDeltaTime = context.DeltaTime / GetAttackDuration(enemy.Type);
    enemy.ActionTick += relativeDeltaTime;
    enemy.AtlasIndex = GetAtlasIndex(enemy.Type) + TextureOffsets::Enemy_Attack;
    
    if (enemy.ActionTick >= GetAttackTiming(enemy.Type) && enemy.ActionTick - relativeDeltaTime < GetAttackTiming(enemy.Type)) {
        glm::vec2 direction = glm::normalize(context.PlayerPosition - enemy.Position);
        context.Areas.emplace_back(enemy.Position, enemy.Position + GetAttackRange(enemy.Type) * direction);
    
        const std::span<LineCollider> area = { context.Areas.end() - 1, 1 };
        const float thickness = 0.25f;
        context.Attacks.emplace_back(area, thickness, GetAttackDamage(enemy.Type));

        enemy.Tick++;
    }

    if (enemy.ActionTick >= 1.0f || enemy.Health <= 0.0f) {
        enemy.ActionTick = 0.0f;
        return ActionStatus::Done;
    }
    
    return ActionStatus::Running;
}

ActionStatus BasicPathfind(Context& context, Enemy& enemy) {
    enemy.Tick += context.DeltaTime * 2.0f;
    enemy.AtlasIndex = GetAtlasIndex(enemy.Type) + TextureOffsets::Enemy_Walk;
   
    glm::vec2 movementVector = Pathfind(context.AproachMap, enemy.Position, context.Map);

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

ActionStatus BasicDead(Context& context, Enemy& enemy) {
    enemy.AtlasIndex = GetAtlasIndex(enemy.Type) + TextureOffsets::Enemy_Corpse;
    return ActionStatus::Done;
}

ActionStatus RangedPathfind(Context& context, Enemy& enemy) {
    enemy.Tick += context.DeltaTime * 2.0f;
    enemy.ActionTick += context.DeltaTime;
    enemy.AtlasIndex = GetAtlasIndex(enemy.Type) + TextureOffsets::Enemy_Walk;
   
    glm::vec2 movementVector = Pathfind(context.RangedApproachMap, enemy.Position, context.Map);
    bool changeDirection = false;

    if (glm::dot(movementVector, movementVector) > 1.0f) { // Use squared distance to save a sqrt
        changeDirection = enemy.ActionTick >= 0.25f;
        movementVector += glm::vec2{ 0.5f, 0.5f } - glm::fract(enemy.Position);
    } else {
        // Circle around player
        glm::vec2 delta = enemy.Position - context.PlayerPosition;
        float direction = 1.0f - (enemy.ActionState == 0) * 2.0f;
        movementVector = { direction * delta.y, -direction * delta.x };
        
        // Move toward optimal range
        float optimalDistance = (glm::dot(delta, delta) - (4.25f * 4.25f)); // Use squared distance to save a sqrt
        movementVector -= optimalDistance * 0.1f * delta;
    }
    
    glm::vec2 collision = Collision(enemy, context.Map, context.Enemies);
    if (collision.x != 0.0f || collision.y != 0.0f) {
        changeDirection = enemy.ActionTick >= 0.25f;
        collision = glm::normalize(collision);
    }

    if (changeDirection) {
        enemy.ActionTick = 0.0f;
        enemy.ActionState = (enemy.ActionState == 0);
    }
    
    movementVector = glm::normalize(movementVector);
    movementVector += collision;
    movementVector *= glm::min(context.DeltaTime.GetSeconds(), 1.0f) * GetSpeed(enemy.Type);
    
    auto oldPosition = enemy.Position;
    enemy.Position += movementVector;
    
    context.UpdateDjikstraMap |= glm::floor(oldPosition) != glm::floor(enemy.Position);

    return ActionStatus::Done;
}