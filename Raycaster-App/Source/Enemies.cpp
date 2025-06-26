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
    m_Map.resize(map.GetSize());
    for (size_t i = 0; i < map.GetSize(); i++) {
        m_Map[i] = map[i];
    }

    m_MapCenter = { map.GetWidth(), map.GetHeight() };
    m_MapCenter *= 0.5f;

    m_MapScale = map.GetScale();
}

void Enemies::Add(EnemyType type, glm::vec2 position) {
    RC_ASSERT(m_Map.size(), "Enemies must be initialized before calling Add");
    m_Enemies.emplace_back(position, 0.0f, GetAtlasIndex(type), type);
}

void Enemies::Update(Core::Timestep deltaTime, const Map& map, glm::vec2 playerPosition) {
    for (size_t i = 0; i < Count(); i++) {
        Enemy& enemy = m_Enemies[i];

        uint32_t mapIndex = (uint32_t)enemy.Position.y * map.GetWidth() + (uint32_t)enemy.Position.x;
        m_Map[mapIndex] = map[mapIndex];

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
            
            bool lineOfSight = true;

            glm::vec2 movementVector = playerPosition;

            glm::vec2 enemyPos(0.0f);
            glm::vec3 scale = GetScale(enemy.Type);
            static const glm::i32vec2 directions[] = {
                glm::i32vec2(1,0),
                glm::i32vec2(-1,0),
                glm::i32vec2(0,1),
                glm::i32vec2(0,-1),
                glm::i32vec2(1,1),
                glm::i32vec2(-1,1),
                glm::i32vec2(1,-1),
                glm::i32vec2(-1,-1),
            };
            for (uint32_t j = 0; j < 8 && lineOfSight; j++) {
                
                enemyPos.x = enemy.Position.x + 0.5f * scale.x * directions[j].x;
                enemyPos.y = enemy.Position.y + 0.5f * scale.y * directions[j].y;

                lineOfSight &= map.LineOfSight(enemyPos, playerPosition);
            }

            if (!lineOfSight) {
                enemyPos.x = enemy.Position.x;
                enemyPos.y = enemy.Position.y;

                movementVector = Algorithms::AStar(enemyPos, playerPosition, m_Map, map.GetWidth(), map.GetHeight());
                movementVector += 0.5f; //Pathfind to tile centre
            }

            movementVector.x -= enemy.Position.x;
            movementVector.y -= enemy.Position.y;
            movementVector = glm::normalize(movementVector);
            movementVector *= deltaTime * GetSpeed(enemy.Type);

            enemy.Position.x += movementVector.x;
            enemy.Position.y += movementVector.y;
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

glm::vec3 Enemy::Scale() const {
    return GetScale(Type);
}