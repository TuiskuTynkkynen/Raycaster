#pragma once
#include "Enemies.h"

struct Context {
    Core::Timestep DeltaTime;
    bool UpdateDjikstraMap;
    glm::vec2 PlayerPosition;
    const Map& Map;
    const std::vector<float>& AproachMap;
    const std::vector<Enemy>& Enemies;
};

enum class ActionStatus : uint8_t {
    Done = 0,
    Running,
};

using Action = ActionStatus(*)(Context&, Enemy&);
using Condition = bool(*)(const Context&, Enemy&);

struct Transition {
    EnemyState::Enumeration CurrentState;
    EnemyState::Enumeration NextState;
    Condition ShouldTransition;
};

///////////////////////////////////////////////////////////////////////////////
//                                Conditions                                 //
///////////////////////////////////////////////////////////////////////////////
template<float distance, bool negate = false>
bool DistanceCondition(const Context& context, Enemy& enemy) {
    return (glm::length(enemy.Position - context.PlayerPosition) < distance) ^ negate;
}

///////////////////////////////////////////////////////////////////////////////
//                                  Actions                                  //
///////////////////////////////////////////////////////////////////////////////
ActionStatus BasicAttack(Context& context, Enemy& enemy);
ActionStatus BasicPathfind(Context& context, Enemy& enemy);

///////////////////////////////////////////////////////////////////////////////
//                        Enemy state machine tables                         //
///////////////////////////////////////////////////////////////////////////////
inline constinit std::array<Action, EnemyState::ENUMERATION_MAX + 1> s_BasicActions = [] {
    std::array<Action, EnemyState::ENUMERATION_MAX + 1> actions;
    actions[EnemyState::Pathfind] = BasicPathfind;
    actions[EnemyState::Attack] = BasicAttack;
    return actions;
    }();

inline constexpr std::array s_BasicTransitions{
    Transition { EnemyState::Pathfind,  EnemyState::Attack,     DistanceCondition<1.1f> },
    Transition { EnemyState::Attack,    EnemyState::Pathfind,   DistanceCondition<1.1f, true> },
};

///////////////////////////////////////////////////////////////////////////////
//                     Enemy type parameters and getters                     //
///////////////////////////////////////////////////////////////////////////////
struct EnemyParameters {
    std::span<const Action> ActionTable;
    std::span<const Transition> TransitionTable;

    glm::vec3 Scale;

    float Speed;
    uint32_t AtlasIndex;
};

inline constinit std::array<EnemyParameters, EnemyType::ENUMERATION_MAX + 1> s_EnemyParameters = [] {
    std::array<EnemyParameters, EnemyType::ENUMERATION_MAX + 1> params;
    params[EnemyType::Basic] = EnemyParameters{
        .ActionTable = s_BasicActions,
        .TransitionTable = s_BasicTransitions,

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

constexpr Action GetAction(EnemyType::Enumeration type, EnemyState::Enumeration state) {
    if (type > EnemyType::ENUMERATION_MAX || state > EnemyState::ENUMERATION_MAX) {
        return nullptr;
    }

    return s_EnemyParameters[type].ActionTable[state];
}

constexpr std::span<const Transition> GetTransitionTable(EnemyType::Enumeration type) {
    if (type > EnemyType::ENUMERATION_MAX) {
        return {};
    }

    return s_EnemyParameters[type].TransitionTable;
}
