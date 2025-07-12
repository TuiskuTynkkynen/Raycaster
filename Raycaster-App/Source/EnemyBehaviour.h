#pragma once
#include "Enemies.h"

///////////////////////////////////////////////////////////////////////////////
//                     Enemy type parameters and getters                     //
///////////////////////////////////////////////////////////////////////////////
struct EnemyParameters {
    glm::vec3 Scale;

    float Speed;
    uint32_t AtlasIndex;
};

inline constinit std::array<EnemyParameters, EnemyType::ENUMERATION_MAX + 1> s_EnemyParameters = [] {
    std::array<EnemyParameters, EnemyType::ENUMERATION_MAX + 1> params;
    params[EnemyType::Basic] = EnemyParameters{
        .Scale{0.8f},
        .Speed{1.0f},
        .AtlasIndex{11},
    };
    return params;
    }();

constexpr glm::vec3 GetScale(EnemyType::Enumeration type) {
    if (type > EnemyType::ENUMERATION_MAX) {
        return glm::vec3(1.0f);
    }

    return s_EnemyParameters[type].Scale;
}

constexpr float GetSpeed(EnemyType::Enumeration type) {
    if (type > EnemyType::ENUMERATION_MAX) {
        return 0.0f;
    }

    return s_EnemyParameters[type].Speed;
}

constexpr uint32_t GetAtlasIndex(EnemyType::Enumeration type) {
    if (type > EnemyType::ENUMERATION_MAX) {
        return 0;
    }

    return s_EnemyParameters[type].AtlasIndex;
}