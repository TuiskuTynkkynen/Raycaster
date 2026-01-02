#pragma once

#include "Map.h"
#include "Entities.h"
#include "Renderables.h"

#include <glm/glm.hpp>

#include <span>

namespace ProjectileType {
    enum Enumeration : uint8_t{
        Dart = 0,
        ENUMERATION_MAX = Dart ,
    };
}

struct Projectile {
    ProjectileType::Enumeration Type;

    glm::vec2 Position;
    glm::vec2 Velocity;

    float Damage;
};

class Projectiles {
public:
    void Init();
    void Shutdown();

    void Add(ProjectileType::Enumeration type, glm::vec2 position, glm::vec2 velocity);
    void Remove(size_t index);
   
    void Update(Core::Timestep deltaTime, const Map& map);
    void UpdateRender(Renderables& renderables, glm::vec2 playerPosition);

    inline size_t Count() const { return m_Projectiles.size(); };
    inline constexpr std::span<const Projectile> Get() const { return m_Projectiles; };
    inline const Projectile& operator[](size_t index) const { 
        RC_ASSERT(index < Count());
        return m_Projectiles[index]; 
    }
private:
    std::vector<Projectile> m_Projectiles;
};