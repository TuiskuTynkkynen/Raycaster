#include <EnemyBehaviour.h>

ActionStatus BasicAttack(Context& context, Enemy& enemy) {
    enemy.Tick += context.DeltaTime;
    enemy.AtlasIndex = GetAtlasIndex(enemy.Type) + 1;
    return ActionStatus::Done;
}

ActionStatus BasicPathfind(Context& context, Enemy& enemy) {
    enemy.Tick += context.DeltaTime * 2.0f;
    enemy.AtlasIndex = GetAtlasIndex(enemy.Type);

    static constexpr size_t directionCount = 8;
    static constexpr std::array<glm::vec2, directionCount + 1> directions = {
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

    glm::vec2 movementVector = glm::vec2{ 0.5f, 0.5f } - glm::fract(enemy.Position);
    {
        glm::vec2 currentPosition = enemy.Position;

        float min = INFINITY;
        for (size_t i = 0; i < 5; i++) {
            size_t dir = directionCount;

            for (size_t j = 0; j < directionCount; j++) {
                size_t x = currentPosition.x + directions[j].x;
                size_t y = currentPosition.y + directions[j].y;
                size_t index = y * context.Map.GetWidth() + x;

                float val = context.AproachMap[index];
                if (val < min) {
                    min = val;
                    dir = j;
                }
            }

            if (dir == directionCount) {
                break;
            }

            currentPosition += directions[dir];
            if (!context.Map.LineOfSight(enemy.Position, currentPosition)) {
                break;
            }

            const float decay = 1.0f / (i * 0.75f + 1.0f);
            movementVector += directions[dir] * decay;
        }
    }

    movementVector = glm::normalize(movementVector);

    glm::vec2 avoid{};
    {
        // TODO fix O(n^2) 
        for (size_t i = 0; i < context.Enemies.size(); i++) {
            if (&enemy == &context.Enemies[i]) {
                continue;
            }

            glm::vec2 dir = enemy.Position - context.Enemies[i].Position;

            // Check squared distance to save a sqrt
            if (glm::dot(dir, dir) > (0.45f * 0.45f)) {
                continue;
            }

            avoid += dir;
        }

        if (avoid.x != 0.0f || avoid.y != 0.0f) {
            avoid = glm::normalize(avoid);
        }

        size_t index = glm::floor(enemy.Position.y) * context.Map.GetWidth() + enemy.Position.x;
        auto adjacent = context.Map.GetNeighbours(index);

        if (adjacent.Bitboard) {
            glm::vec2 fraction = glm::fract(enemy.Position);

            for (size_t j = 0; j < directionCount; j++) {
                if (!adjacent[j]) {
                    continue;
                }

                glm::vec2 dist(0.0f);
                dist.x += (directions[j].x < 0) * 1.0f + directions[j].x * fraction.x;
                dist.y += (directions[j].y < 0) * 1.0f + directions[j].y * fraction.y;

                // Check squared distance to save a sqrt
                if (glm::dot(dist, dist) > (0.45f * 0.45f)) {
                    continue;
                }

                avoid += directions[j];
            }
        }
    }

    movementVector += avoid;
    if (movementVector.x != 0.0f || movementVector.y != 0.0f) {
        movementVector = glm::normalize(movementVector);
    }

    movementVector *= glm::min(context.DeltaTime.GetSeconds(), 1.0f) * GetSpeed(enemy.Type);
    
    auto oldPosition = enemy.Position;
    enemy.Position += movementVector;

    context.UpdateDjikstraMap |= glm::floor(oldPosition) != glm::floor(enemy.Position);

    return ActionStatus::Done;
}