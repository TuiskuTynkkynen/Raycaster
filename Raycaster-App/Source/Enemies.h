#pragma once
#include "Map.h"
#include "Entities.h"
#include "SpatialPartition.h"

#include "Core.h"

#include <vector>
#include <span>

namespace EnemyType {
    enum Enumeration : uint8_t {
        Basic = 0,
        Ranged,
        ENUMERATION_MAX = Ranged,
    };
}

namespace EnemyState {
    enum Enumeration : uint8_t {
        Pathfind = 0,
        Attack,
        Dead,
        ENUMERATION_MAX = Dead
    };
}

struct Enemy {
    glm::vec2 Position{};
    float Health = 0.0f;
    float Tick = 0.0f;
    float ActionTick = 0.0f;

    uint32_t AtlasIndex = 0;
    EnemyType::Enumeration Type;
    EnemyState::Enumeration State;
    uint8_t ActionState;

    glm::vec3 Scale() const;
};

class Enemies {
public:
    Enemies() {};

    void Init(const Map& map);
    void Shutdown();

    void Add(EnemyType::Enumeration type, glm::vec2 position);

    void DamageAreas(std::span<const LineCollider> areas, float areaThickness, float damage);

    void Update(Core::Timestep deltaTime, const Map& map, glm::vec2 playerPosition);

    void UpdateRender(std::span<Tile> tiles, std::span<Sprite> sprites, std::span<Core::Model> models);

    inline size_t Count() const { return m_Enemies.size(); }

    inline const std::span<const Enemy> Get() const { return m_Enemies; }
    const Enemy& operator [](size_t index) const {
        RC_ASSERT(index < Count());
        return m_Enemies[index];
    }
    struct Attack {
        std::span<LineCollider> Areas;
        float Thickness;
        float Damage;
    };
    inline const std::span<const Attack> GetAttacks() const { return m_Attacks;  };
private:
    void UpdateApproachMap(const Map& map, glm::ivec2 playerPosition);
    void UpdateRangedApproachMap(const Map& map, glm::vec2 playerPosition);
    void UpdateCostMap(const Map& map);
    
    std::vector<Enemy> m_Enemies;

    glm::ivec2 m_PreviousPlayerPosition{};
    std::vector<float> m_ApproachMap;
    std::vector<float> m_RangedApproachMap;
    std::vector<float> m_CostMap;

    SpatialPartition<Enemy, uint16_t> m_SpatialPartition;

    std::vector<LineCollider> m_Areas;
    std::vector<Attack> m_Attacks;

    std::vector<std::pair<glm::ivec3, float>> m_Frontier;

    glm::vec2 m_MapCenter{};
    glm::vec2 m_MapScale{};
};