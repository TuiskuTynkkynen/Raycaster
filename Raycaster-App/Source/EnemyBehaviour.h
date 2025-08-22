#pragma once
#include "Enemies.h"

struct Context {
    Core::Timestep DeltaTime;
    bool UpdateDjikstraMap;
    glm::vec2 PlayerPosition;
    const Map& Map;
    std::vector<LineCollider>& Areas;
    std::vector<Enemies::Attack>& Attacks;
    const std::vector<float>& AproachMap;
    const std::vector<float>& RangedApproachMap;
    const std::vector<Enemy>& Enemies;
    SpatialPartition<Enemy, uint16_t>& SpatialPartition;
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
template<Condition A, Condition... B>
bool And(const Context& context, Enemy& enemy){
    return (A(context, enemy) && And<B...>(context, enemy));
}

template<typename=void>
bool And(const Context& context, Enemy& enemy){
    return true;
}

bool LineOfSight(const Context& context, glm::vec2 start, glm::vec2 end);

template<bool negate = false>
bool LineOfSightCondtion(const Context& context, Enemy& enemy){
    return LineOfSight(context, enemy.Position, context.PlayerPosition) ^ negate;
}

template<float distance, bool negate = false>
bool DistanceCondition(const Context& context, Enemy& enemy) {
    return (glm::length(enemy.Position - context.PlayerPosition) < distance) ^ negate;
}

template<float min, float max, bool negate = false>
bool RangeCondition(const Context& context, Enemy& enemy) {
    float distance = glm::length(enemy.Position - context.PlayerPosition);
    return ((min < distance) && (distance < max)) ^ negate;
}

template<float health, bool negate = false>
bool HealthCondition(const Context& context, Enemy& enemy) {
    return (enemy.Health <= health) ^ negate;
}

///////////////////////////////////////////////////////////////////////////////
//                                  Actions                                  //
///////////////////////////////////////////////////////////////////////////////
ActionStatus BasicAttack(Context& context, Enemy& enemy);
ActionStatus BasicPathfind(Context& context, Enemy& enemy);
ActionStatus BasicDead(Context& context, Enemy& enemy);
ActionStatus RangedPathfind(Context& context, Enemy& enemy);

///////////////////////////////////////////////////////////////////////////////
//                        Enemy state machine tables                         //
///////////////////////////////////////////////////////////////////////////////
inline constinit std::array<Action, EnemyState::ENUMERATION_MAX + 1> s_BasicActions = [] {
    std::array<Action, EnemyState::ENUMERATION_MAX + 1> actions;
    actions[EnemyState::Pathfind] = BasicPathfind;
    actions[EnemyState::Attack] = BasicAttack;
    actions[EnemyState::Dead] = BasicDead;
    return actions;
    }();

inline constexpr std::array s_BasicTransitions{
    Transition { EnemyState::Pathfind,  EnemyState::Attack,     DistanceCondition<1.1f> },
    Transition { EnemyState::Attack,    EnemyState::Pathfind,   DistanceCondition<1.1f, true> },
    Transition { EnemyState::Pathfind,  EnemyState::Dead,     HealthCondition<0.0f> },
    Transition { EnemyState::Attack,    EnemyState::Dead,   HealthCondition<0.0f> },
};

inline constinit std::array<Action, EnemyState::ENUMERATION_MAX + 1> s_RangedActions = [] {
    std::array<Action, EnemyState::ENUMERATION_MAX + 1> actions;
    actions[EnemyState::Pathfind] = RangedPathfind;
    actions[EnemyState::Attack] = BasicAttack;
    actions[EnemyState::Dead] = BasicDead;
    return actions;
    }();

inline constexpr std::array s_RangedTransitions{
    Transition { EnemyState::Pathfind,  EnemyState::Attack,     RangeCondition<3.25f, 4.75f> },
    Transition { EnemyState::Attack,    EnemyState::Pathfind,   RangeCondition<3.25f, 4.75f, true> },
    Transition { EnemyState::Pathfind,  EnemyState::Dead,     HealthCondition<0.0f> },
    Transition { EnemyState::Attack,    EnemyState::Dead,   HealthCondition<0.0f> },
};

///////////////////////////////////////////////////////////////////////////////
//                     Enemy type parameters and getters                     //
///////////////////////////////////////////////////////////////////////////////
struct EnemyParameters {
    std::span<const Action> ActionTable;
    std::span<const Transition> TransitionTable;

    glm::vec3 Scale{};

    float Speed{};
    float AttackDamage{};
    float AttackRange{};
    float AttackDuration{};
    float AttackTiming{};
    uint32_t AtlasIndex{};
};

inline constinit std::array<EnemyParameters, EnemyType::ENUMERATION_MAX + 1> s_EnemyParameters = [] {
    std::array<EnemyParameters, EnemyType::ENUMERATION_MAX + 1> params;
    params[EnemyType::Basic] = EnemyParameters{
        .ActionTable = s_BasicActions,
        .TransitionTable = s_BasicTransitions,

        .Scale{0.8f},
        .Speed{1.0f},
        .AttackDamage{1.0f},
        .AttackRange{1.25f},
        .AttackDuration{1.5f},
        .AttackTiming{1.0f / 1.5f},
        .AtlasIndex{TextureIndices::Enemy_Basic},
    };
    params[EnemyType::Ranged] = EnemyParameters{
        .ActionTable = s_RangedActions,
        .TransitionTable = s_RangedTransitions,

        .Scale{0.8f},
        .Speed{1.0f},
        .AttackDamage{1.0f},
        .AttackRange{0.0f},
        .AttackDuration{1.0f},
        .AttackTiming{1.0f},
        .AtlasIndex{TextureIndices::Enemy_Basic},
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

constexpr float GetAttackDamage(EnemyType::Enumeration type) {
    if (type > EnemyType::ENUMERATION_MAX) {
        return 0.0f;
    }

    return s_EnemyParameters[type].AttackDamage;
}

constexpr float GetAttackRange(EnemyType::Enumeration type) {
    if (type > EnemyType::ENUMERATION_MAX) {
        return 0.0f;
    }

    return s_EnemyParameters[type].AttackRange;
}

constexpr float GetAttackDuration(EnemyType::Enumeration type) {
    if (type > EnemyType::ENUMERATION_MAX) {
        return 0.0f;
    }

    return s_EnemyParameters[type].AttackDuration;
}

constexpr float GetAttackTiming(EnemyType::Enumeration type) {
    if (type > EnemyType::ENUMERATION_MAX) {
        return 0.0f;
    }

    return s_EnemyParameters[type].AttackTiming;
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
