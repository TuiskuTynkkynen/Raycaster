#include "Enemies.h"

#include "EnemyBehaviour.h"
#include "Algorithms.h"

void Enemies::Init(const Map& map) {
    m_Frontier.resize(map.GetSize());

    m_MapCenter = { map.GetWidth(), map.GetHeight() };
    m_MapCenter *= 0.5f;

    m_MapScale = map.GetScale();
}

void Enemies::Add(EnemyType::Enumeration type, glm::vec2 position) {
    RC_ASSERT(m_Frontier.size(), "Enemies must be initialized before calling Add");
    m_Enemies.emplace_back(position, 0.0f, GetAtlasIndex(type), type);
}

void Enemies::Update(Core::Timestep deltaTime, const Map& map, glm::vec2 playerPosition) {
    static bool shouldUpdate = false;

    if (shouldUpdate || m_PreviousPlayerPosition != glm::ivec2(playerPosition)) {
    UpdateApproachMap(map, playerPosition);
        UpdateRangedApproachMap(map, playerPosition);
        shouldUpdate = false;
    }
    m_PreviousPlayerPosition = playerPosition;

    Context context{
        .DeltaTime = deltaTime,
        .UpdateDjikstraMap = false,
        .PlayerPosition = playerPosition,
        .Map = map,
        .AproachMap = m_ApproachMap,
        .RangedApproachMap = m_RangedApproachMap,
        .Enemies = m_Enemies
    };

    for (size_t i = 0; i < Count(); i++) {
        Enemy& enemy = m_Enemies[i];

        auto action = GetAction(enemy.Type, enemy.State);
        if (action(context, enemy) == ActionStatus::Running) {
                        continue;
                    }

        auto transitionTable = GetTransitionTable(enemy.Type);
        for (auto& Transition : transitionTable) {
            if (Transition.CurrentState != enemy.State || !Transition.ShouldTransition(context, enemy)) {
                        continue;
                    }

            enemy.State = Transition.NextState;
            break;
                }
                }

    shouldUpdate |= context.UpdateDjikstraMap;
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
        glm::vec2 index = glm::vec2((enemy.AtlasIndex) % ATLASWIDTH, (enemy.AtlasIndex) / ATLASWIDTH);
        models[i].Materials.front()->Parameters.back().Value = glm::vec2(flip ? 0.0f : 1.0f, 0.0f);
        models[i].Materials.front()->Parameters.front().Value = index;
    }
}

static std::vector<float> CreateDjikstraMap(std::span<glm::ivec2> destinations, const std::vector<float>& costMap, const Map& map, std::vector<std::pair<glm::ivec3, float>>& frontier) {
    std::vector<float> result(map.GetSize(), std::numeric_limits<float>::infinity());

    constexpr float sqrt2 = 0.4142135624f;
    constexpr size_t directionCount = 4;
    constexpr std::array<glm::ivec2, directionCount + 1> directions = {
        glm::ivec2(-1.0f,  0.0f),
        glm::ivec2(1.0f,  0.0f),
        glm::ivec2(0.0f, -1.0f),
        glm::ivec2(0.0f,  1.0f),
        glm::ivec2(0.0f,  0.0f),
    };

    constexpr size_t mapSize = Map::GetSize();

    size_t back = destinations.size();
    for (size_t i = 0; i < back; i++) {
        auto destination = destinations[i];
        size_t index = map.GetIndex(destination);

    if (index < map.GetSize()) {
            frontier[i] = { glm::ivec3{ destination, directionCount}, costMap[index] };
        result[index] = 0.0f;
    }
    }

    while (back) {
        auto& front = frontier[--back];

        glm::ivec2 current = front.first;
        int32_t history = front.first.z;

        float value = front.second;

        for (size_t i = 0; i < directionCount; i++) {
            glm::ivec2 next = current + directions[i];

            size_t index = map.GetIndex(next);

            bool diagonal = (i < 2) ^ (history < 2) && (history != directionCount);
            float nextValue = value + (diagonal ? sqrt2 : 1.0f) + costMap[index];
            if (index >= mapSize
                || result[index] <= nextValue
                || map[index]) {
                continue;
            }

            result[index] = nextValue;

            auto& front = frontier[back++];
            front.first = glm::ivec3{ next, (diagonal ? directionCount : i) };
            front.second = nextValue;
        }
        }

    return result;
    }

void Enemies::UpdateApproachMap(const Map& map, glm::ivec2 playerPosition) {
    UpdateCostMap(map);
    std::array arr = { playerPosition };
    m_ApproachMap = CreateDjikstraMap(arr, m_CostMap, map, m_Frontier);
}


void Enemies::UpdateRangedApproachMap(const Map& map, glm::vec2 playerPosition) {
    UpdateCostMap(map);
    
    auto destinations = Algorithms::MidpointCicle(glm::ivec2(playerPosition), 4);
    auto last = std::remove_if(destinations.begin(), destinations.end(), [map, playerPosition](glm::vec2 pos) {
        size_t index = map.GetIndex(pos);
        pos += 0.5f;
        return index >= map.GetSize() || map[index] || !map.LineOfSight(playerPosition, pos);
        });

    m_RangedApproachMap = CreateDjikstraMap({destinations.begin(), last}, m_CostMap, map, m_Frontier);
}

void Enemies::UpdateCostMap(const Map& map) {
    m_CostMap.assign(map.GetSize(), 0.0f);

    for (const auto& enemy : m_Enemies) {
        size_t enemyIndex = map.GetIndex(enemy.Position);

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

glm::vec3 Enemy::Scale() const {
    return GetScale(Type);
}