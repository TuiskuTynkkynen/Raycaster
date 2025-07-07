#include "Enemies.h"

static constexpr glm::vec3 GetScale(EnemyType type) {
    switch (type) {
    case EnemyType::Basic:
        return glm::vec3(0.8f);
    }
}

static constexpr float GetSpeed(EnemyType type) {
    switch (type) {
    case EnemyType::Basic:
        return 1.0f;
    }
}

static constexpr uint32_t GetAtlasIndex(EnemyType type) {
    switch (type) {
    case EnemyType::Basic:
        return 11;
    }
}

void Enemies::Init(const Map& map) {
    m_Frontier.resize(map.GetSize());

    m_MapCenter = { map.GetWidth(), map.GetHeight() };
    m_MapCenter *= 0.5f;

    m_MapScale = map.GetScale();
}

void Enemies::Add(EnemyType type, glm::vec2 position) {
    RC_ASSERT(m_Frontier.size(), "Enemies must be initialized before calling Add");
    m_Enemies.emplace_back(position, 0.0f, GetAtlasIndex(type), type);
}

void Enemies::Update(Core::Timestep deltaTime, const Map& map, glm::vec2 playerPosition) {
    bool updated = false;

    if (m_PreviousPlayerPosition != glm::ivec2(playerPosition)) {
        updated = true;
    UpdateApproachMap(map, playerPosition);
    }
    m_PreviousPlayerPosition = playerPosition;

    for (size_t i = 0; i < Count(); i++) {
        Enemy& enemy = m_Enemies[i];

        glm::vec2 distance = enemy.Position - playerPosition;
        if (glm::length(distance) < 1.1f) {
            //"Attack"
            enemy.Tick += deltaTime;
            enemy.AtlasIndex = GetAtlasIndex(enemy.Type) + 1;
            int i = 0;
        } else {
            //Pathfinding
            enemy.Tick += deltaTime * 2.0f;
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

                for (size_t i = 0; i < 5; i++) {
            float min = INFINITY;
            size_t dir = directionCount;

                    for (size_t j = 0; j < directionCount; j++) {
                        size_t x = currentPosition.x + directions[j].x;
                        size_t y = currentPosition.y + directions[j].y;
                size_t index = y * map.GetWidth() + x;

                float val = m_ApproachMap[index];
                if (val < min) {
                    min = val;
                    dir = j;
            }
                    }

                    currentPosition += directions[dir];
                    if (!map.LineOfSight(enemy.Position, currentPosition)) {
                        break;
                    }
                    movementVector += directions[dir];
            }   
            }

            movementVector = glm::normalize(movementVector);

            glm::vec2 avoid{};
            {
                // TODO fix O(n^2) 
                for (size_t j = 0; j < Count(); j++) {
                    if (j == i) {
                        continue;
                    }

                    glm::vec2 dir = enemy.Position - m_Enemies[j].Position;
                    
                    // Check squared distance to save a sqrt
                    if (glm::dot(dir, dir) > (0.45f * 0.45f)) {
                        continue;
                    }

                    avoid += dir;
                }

                if (avoid.x != 0.0f || avoid.y != 0.0f) {
                    avoid= glm::normalize(avoid);
                }

                size_t index = glm::floor(enemy.Position.y) * map.GetWidth() + enemy.Position.x;
                auto adjacent = map.GetNeighbours(index);

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

            movementVector *= glm::min(deltaTime.GetSeconds(), 1.0f) * GetSpeed(enemy.Type);
            
            if (!updated && glm::floor(enemy.Position) != glm::floor(enemy.Position + movementVector)) {
                UpdateApproachMap(map, playerPosition);
            }
            
            enemy.Position += movementVector;
        }
    }
}

void Enemies::UpdateRender(std::span<Tile> tiles, std::span<Sprite> sprites, std::span<Core::Model> models) {
    for (size_t i = 0; i < Count(); i++) {
        const Enemy& enemy = m_Enemies[i];
        glm::vec3 scale = GetScale(enemy.Type);

        // Update on Raycaster-layer
        sprites[i].Position.x = enemy.Position.x;
        sprites[i].Position.y = enemy.Position.y;
        sprites[i].Position.z = scale.z * 0.5f;
        sprites[i].WorldPosition = sprites[i].Position;

        sprites[i].Scale = scale;
        sprites[i].AtlasIndex = enemy.AtlasIndex;

        bool flip = (uint32_t)enemy.Tick % 2 == 0;
        sprites[i].FlipTexture = flip;

        //Update on 2D-layer
        tiles[i].Posistion.x = (enemy.Position.x - m_MapCenter.x) * m_MapScale.x;
        tiles[i].Posistion.y = (m_MapCenter.y - enemy.Position.y) * m_MapScale.y;

        //Update on 3D-layer
        uint32_t atlasWidth = 11;
        glm::vec2 index = glm::vec2((enemy.AtlasIndex) % atlasWidth, (enemy.AtlasIndex) / atlasWidth);
        models[i].Materials.front()->Parameters.back().Value = glm::vec2(flip ? 0.0f : 1.0f, 0.0f);
        models[i].Materials.front()->Parameters.front().Value = index;
    }
}

void Enemies::UpdateApproachMap(const Map& map, glm::ivec2 playerPosition) {
    UpdateCostMap(map);
    m_ApproachMap.assign(map.GetSize(), INFINITY);

    constexpr float sqrt2 = 0.4142135624f;
    constexpr size_t directionCount = 4;
    constexpr std::array<glm::ivec2, directionCount + 1> directions = {
        glm::ivec2(-1.0f,  0.0f),
        glm::ivec2(1.0f,  0.0f),
        glm::ivec2(0.0f, -1.0f),
        glm::ivec2(0.0f,  1.0f),
        glm::ivec2(0.0f,  0.0f),
    };

    size_t back = 1;
    m_Frontier[0] = { glm::ivec3{ playerPosition, directionCount }, 0.0f };

    size_t mapSize = map.GetSize();
    size_t mapWidth = map.GetWidth();

    m_ApproachMap[playerPosition.y * mapWidth + playerPosition.x] = 0;

    while (back) {
        auto& frontier = m_Frontier[--back];

        glm::ivec2 current = frontier.first;
        int32_t history = frontier.first.z;

        float value = frontier.second;

        for (size_t i = 0; i < directionCount; i++) {
            glm::ivec2 next = current + directions[i];

            size_t index = next.y * mapWidth + next.x;

            bool diagonal = (i < 2) ^ (history < 2) && (history != directionCount);
            float nextValue = value + (diagonal ? sqrt2 : 1.0f) + m_CostMap[index];
            if (index >= mapSize
                || m_ApproachMap[index] <= nextValue
                || map[index]) {
                continue;
            }

            m_ApproachMap[index] = nextValue;

            auto& frontier = m_Frontier[back++];
            frontier.first = glm::ivec3{ next, (diagonal ? directionCount : i) };
            frontier.second = nextValue;
        }

void Enemies::UpdateCostMap(const Map& map) {
    m_CostMap.assign(map.GetSize(), 0.0f);

    for (const auto& enemy : m_Enemies) {
        size_t enemyIndex = size_t(enemy.Position.y) * map.GetWidth() + enemy.Position.x;

        for (int32_t y = -2; y <= 2; y++) {
            size_t yindex = enemyIndex + y * (int32_t)map.GetWidth();
            for (int32_t x = -2; x <= 2; x++) {
                size_t index = yindex + x;

                if (index >= map.GetSize()) {
                    continue;
                }
        
                m_CostMap[index] += 2 - glm::abs(y) + 2 - glm::abs(x);
        
            }
        }
    }
}

void Enemies::Print() {
    for (size_t i = 0; i < Map::GetSize(); i++) {
        if (i % Map::GetWidth() == 0) {
            std::cout << "\n";
        }

        if (m_ApproachMap[i] == INFINITY) {
            std::cout << " ";
            continue;
        }

        char c = '0' + (char)m_ApproachMap[i];
        std::cout << c;
    }
}

glm::vec3 Enemy::Scale() const {
    return GetScale(Type);
}