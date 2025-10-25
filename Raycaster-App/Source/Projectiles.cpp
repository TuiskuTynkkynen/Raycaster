#include "Projectiles.h"

struct ProjectileParameters {
    float Damage{};
    float Scale{};

    uint32_t AtlasIndex{};
    uint32_t DirectionCount{};
};

inline constinit std::array<ProjectileParameters, ProjectileType::ENUMERATION_MAX + 1> s_ProjectileParameters = [] {
    std::array<ProjectileParameters, ProjectileType::ENUMERATION_MAX + 1> params;
    params[ProjectileType::Dart] = ProjectileParameters{
        .Damage = 1.0f,
        .Scale = 0.25f,
        .AtlasIndex{TextureIndices::Projectile_Dart_First},
        .DirectionCount{TextureIndices::Projectile_Dart_Last - TextureIndices::Projectile_Dart_First},
    };

    return params;
}();

inline constexpr float GetDamage(ProjectileType::Enumeration type) {
    if (type > ProjectileType::ENUMERATION_MAX) {
        return 0.0f;
    }

    return s_ProjectileParameters[type].Damage;
}

inline constexpr float GetScale(ProjectileType::Enumeration type) {
    if (type > ProjectileType::ENUMERATION_MAX) {
        return 0.0f;
    }

    return s_ProjectileParameters[type].Scale;
}

inline constexpr uint32_t GetAtlasIndex(ProjectileType::Enumeration type) {
    if (type > ProjectileType::ENUMERATION_MAX) {
        return 0;
    }

    return s_ProjectileParameters[type].AtlasIndex;
}

struct DotProducts {
    const float View;
    const float Normal;
};

inline constexpr uint32_t GetAtlasOffset(ProjectileType::Enumeration type, DotProducts dots) {
    if (type > ProjectileType::ENUMERATION_MAX) {
        return 0;
    }

    float multiplier = glm::pow(dots.View, 3); //Bias away from 0/180 degree angles
    multiplier = glm::abs(multiplier + 1.0f) * 0.4999f; // Convert into range 0 to 1
    
    RC_ASSERT(multiplier <= 1.0f);

    return (s_ProjectileParameters[type].DirectionCount + 1) * multiplier;
}


inline constexpr bool GetFlipTexture(DotProducts dots) {
    return dots.Normal <= 0.0f;
}

inline DotProducts CalculateDotProducts(glm::vec2 projectilePosition, glm::vec2 projectileVelocity, glm::vec2 viewPosition) {
    glm::vec2 viewDirection = glm::normalize(projectilePosition - viewPosition);
    glm::vec2 projectileDirection = glm::normalize(projectileVelocity);
    glm::vec2 normalDirection(-projectileDirection.y, projectileDirection.x);

    return DotProducts{
        .View = glm::dot(projectileDirection, viewDirection),
        .Normal = glm::dot(normalDirection, viewDirection),
    };
}

void Projectiles::Add(ProjectileType::Enumeration type, glm::vec2 position, glm::vec2 velocity) {
    m_Projectiles.emplace_back(type, position, velocity, GetDamage(type));
}

void Projectiles::Remove(size_t index) {
    if (index >= m_Projectiles.size()) {
        return;
    }

    m_Projectiles[index] = m_Projectiles.back();
    m_Projectiles.pop_back();
}

void Projectiles::Init() {
    m_Projectiles.reserve(16);
}

void Projectiles::Shutdown() {
    m_Projectiles.clear();
    m_Projectiles.shrink_to_fit();
}

void Projectiles::Update(Core::Timestep deltaTime, const Map& map) {
    for (size_t i = 0; i < m_Projectiles.size(); i++) {
        Projectile& projectile = m_Projectiles[i];

        glm::vec2 previousPosition = projectile.Position;
        projectile.Position += projectile.Velocity * deltaTime.GetSeconds();

        size_t mapIndex = map.GetIndex(projectile.Position);
        auto tile = map[mapIndex];
        
        if (!tile) {
            // Didn't collide with a wall
            continue;
        }

        // Check collision with diagonal wall
        if (tile < 0) {
            glm::vec2 wallPoint1 = glm::floor(projectile.Position);
            glm::vec2 wallPoint2(wallPoint1);

            auto adjacent = map.GetNeighbours(mapIndex);
            if (adjacent.South && adjacent.East || adjacent.North && adjacent.West) {
                wallPoint1.x++;
                wallPoint2.y++;
            } else {
                wallPoint2.x++;
                wallPoint2.y++;
            }

            if (!Algorithms::LineIntersection(wallPoint1, wallPoint2, projectile.Position, previousPosition)) {
                // Didn't collide with a wall
                continue;
            }
        }

        Remove(i--);
    }
}

void Projectiles::UpdateRender(Renderables& renderables, glm::vec2 playerPosition) {
    for (size_t i = 0; i < m_Projectiles.size(); i++) {
        auto& sprite = renderables.GetNextSprite();
        auto& model = renderables.GetNextModel();
        Projectile& projectile = m_Projectiles[i];

        glm::vec3 scale(GetScale(projectile.Type));
        
        const DotProducts dots = CalculateDotProducts(projectile.Position, projectile.Velocity, playerPosition);
        uint32_t atlasIndex = GetAtlasIndex(projectile.Type) + GetAtlasOffset(projectile.Type, dots);
        bool flipTexture = GetFlipTexture(dots);

        // Update on Raycaster-layer
        sprite.Position.x = projectile.Position.x;
        sprite.Position.y = projectile.Position.y;
        sprite.Position.z = 0.4f;
        sprite.WorldPosition = sprite.Position;
        sprite.Scale = scale;

        sprite.AtlasIndex = atlasIndex;
        sprite.FlipTexture = flipTexture;

        //Update on 3D-layer
        glm::vec2 index = glm::vec2((atlasIndex) % ATLASWIDTH, (atlasIndex) / ATLASWIDTH);
        model.Materials.front()->Parameters.back().Value = glm::vec2(flipTexture, 0.0f);
        model.Materials.front()->Parameters.front().Value = index;
    }
}
