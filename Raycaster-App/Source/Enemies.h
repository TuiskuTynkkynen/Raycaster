#pragma once
#include "Map.h"
#include "Entities.h"

#include "Core.h"

#include <vector>
#include <span>

enum class EnemyType : uint8_t {
    Basic = 0,
};

struct Enemy {
    glm::vec2 Position{};
    float Tick = 0.0f;

    uint32_t AtlasIndex = 0;
    EnemyType Type;

    glm::vec3 Scale() const;
};

class Enemies {
public:
    Enemies() {};

    void Init(const Map& map);
    void Shutdown() { m_Enemies.clear(); m_Enemies.shrink_to_fit(); m_Map.clear(); m_Map.shrink_to_fit(); }

    void Add(EnemyType type, glm::vec2 position);

    void Update(Core::Timestep deltaTime, const Map& map, glm::vec2 playerPosition);

    void UpdateRender(std::span<Tile> tiles, std::span<Sprite> sprites, std::span<Core::Model> models);

    inline size_t Count() const { return m_Enemies.size(); }

    inline const std::span<const Enemy> Get() const { return m_Enemies; }
    const Enemy& operator [](size_t index) const {
        RC_ASSERT(index < Count());
        return m_Enemies[index];
    }
private:
    std::vector<Enemy> m_Enemies;
    std::vector<bool> m_Map;

    glm::vec2 m_MapCenter{};
    glm::vec2 m_MapScale{};
};