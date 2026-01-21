#include "Player.h"

#include "KeyBinds.h"

#include "Core/Debug/Assert.h"

#include <glm/gtx/euler_angles.hpp>

#include <cmath>
#include <limits>
#include <type_traits>
#include <vector>

void Player::Init(const Map& map) {
    m_Position = glm::vec3((float)map.GetWidth() / 2, (float)map.GetHeight() / 2, 0.5f);
    m_Scale = glm::vec3(Width * 0.5f);
    m_Rotation = glm::vec2(90.0f, 0.0f);

    m_HeldItemIndex = 0;
    m_Inventory[m_HeldItemIndex] = { .Scale = 0.0f, .Count = 0 };
    
    m_Health = MaxHealth;

    m_Areas.reserve(2);
    m_Attacks.reserve(2);
    m_Projectiles.reserve(2);
}

void Player::Shutdown() {
    m_Areas.clear();
    m_Areas.shrink_to_fit();
    
    m_Attacks.clear();
    m_Attacks.shrink_to_fit();

    m_Projectiles.clear();
    m_Projectiles.shrink_to_fit();
}

void Player::Update(std::span<const LineCollider> walls, std::span<const LineCollider> doors, Core::Timestep deltaTime) {
    m_Areas.clear();
    m_Attacks.clear();
    m_Projectiles.clear();

    Move(walls, doors, deltaTime);
    if (m_AnimationProgress >= 0.0f) {
        UseItem(deltaTime);
    }
    m_ShouldInteract = false;
}

void Player::UpdateRender(Renderables& renderables) {
    auto& heldItem = GetHeldItem();

    if (heldItem.Count == 0) {
        return;
    }

    uint32_t atlasIndex = heldItem.UseAnimation.GetFrame(m_AnimationProgress);
    float epsilon = 1e-3f;
    
    // Update on Raycaster-layer
    auto& sprite = renderables.GetNextSprite();
    sprite.Position.x = m_Position.x + glm::cos(glm::radians(m_Rotation.x)) * epsilon;
    sprite.Position.y = m_Position.y - glm::sin(glm::radians(m_Rotation.x)) * epsilon;
    sprite.Position.z = m_Position.z + (glm::tan(glm::radians(m_Rotation.y)) - 1.0f + heldItem.Scale) * epsilon;
    sprite.WorldPosition = sprite.Position;

    sprite.Scale = glm::vec3(2.0f * heldItem.Scale * epsilon);
    sprite.AtlasIndex = atlasIndex;
    sprite.FlipTexture = false;

    //Update on 3D-layer
    RC_ASSERT(renderables.GetStaticModelCount() >= 1);
    auto& model = renderables.GetStaticModels()[1];

    glm::mat4 rotation = glm::eulerAngleYX(glm::radians(m_Rotation.x - 90.f), glm::radians(m_Rotation.y));
    glm::vec3 position3D(m_Position.x, m_Position.z, m_Position.y);
    glm::vec3 offset = rotation * glm::vec4(0.f, -(1.0f - heldItem.Scale) * epsilon, -epsilon, 0.0f);
    
    model.Transform = glm::translate(glm::mat4(1.0f), position3D + offset);
    model.Transform *= rotation;
    model.Transform = glm::scale(model.Transform, sprite.Scale);

    model.Materials.front()->Parameters.back().Value = glm::vec2(0.0f, 0.0f);
    glm::vec2 index = glm::vec2((atlasIndex) % ATLASWIDTH, (atlasIndex) / ATLASWIDTH);
    model.Materials.front()->Parameters.front().Value = index;
}

bool Player::DamageAreas(std::span<const LineCollider> attack, float thickness, float damage) {
    const bool hit = Algorithms::LineCollisions(m_Position, attack, thickness + Width * 0.5f) != glm::vec2(0.0f);
    m_Health -= damage * hit;

    return hit;
}

void Player::PickUp(Item item) {
    m_HeldItemIndex = item.AdditionalData.index() + 1;
    RC_ASSERT(m_HeldItemIndex < m_Inventory.size());

    m_Inventory[m_HeldItemIndex] = item;
}

void Player::SwitchItem(size_t index) {
    RC_ASSERT(index < m_Inventory.size());
    if (index == 0 || m_Inventory[index].Count > 0) {
        m_HeldItemIndex = index;
    }
}

bool Player::OnKeyEvent(Core::KeyPressed event) {
    bool HoldingItem = m_HeldItemIndex < m_Inventory.size() && m_Inventory[m_HeldItemIndex].UseDuration;
    bool UsingItem = HoldingItem && m_AnimationProgress >= 0.0f;

    std::optional<KeyBinds::Name> action = KeyBinds::KeyCodeToKeyBind(event.GetKeyCode());
    if (!action) {
        return false;
    }

    switch (action.value()) {
        using enum KeyBinds::Name;
    case Forward:
        m_LateralSpeed.x = 1.0f;
        return true;
    case Backward:
        m_LateralSpeed.x = -1.0f;
        return true;
    case Left:
        m_LateralSpeed.y = -1.0f;
        return true;
    case Right:
        m_LateralSpeed.y = 1.0f;
        return true;
    case LookLeft:
        m_RotationalSpeed.x = 1.0f;
        return true;
    case LookRight:
        m_RotationalSpeed.x = -1.0f;
        return true;
    case LookUp:
        m_RotationalSpeed.y = 1.0f;
        return true;
    case LookDown:
        m_RotationalSpeed.y = -1.0f;
        return true;
    case UseItem:
        if (HoldingItem && !UsingItem) {
            m_AnimationProgress = 0.0f;
        }
        return true;
    case Item0:
    case Item1:
    case Item2:
        SwitchItem(action.value() - Item0);
        return true;
    case Interact:
        m_ShouldInteract = !event.IsRepeated();
        return true;
    }

    return false;
}


bool Player::OnKeyEvent(Core::KeyReleased event) {
    std::optional<KeyBinds::Name> action = KeyBinds::KeyCodeToKeyBind(event.GetKeyCode());
    if (!action) {
        return false;
    }

    switch (action.value()) {
        using enum KeyBinds::Name;
    case Forward:
    case Backward:
        m_LateralSpeed.x = 0.0f;
        return true;
    case Left:
    case Right:
        m_LateralSpeed.y = 0.0f;
        return true;
    case LookLeft:
    case LookRight:
        m_RotationalSpeed.x = 0.0f;
        return true;
    case LookUp:
    case LookDown:
        m_RotationalSpeed.y = 0.0f;
        return true;
    default:
        return false;
    }
}

void Player::Move(std::span<const LineCollider> walls, std::span<const LineCollider> doors, Core::Timestep deltaTime) {
    const float MaxLateralSpeed = 2.0f;
    const float MaxRotationalSpeed = 180.0f;

    
    const float speed = glm::length(m_LateralSpeed);
    if (speed != 0.0f) {
        const float cos = glm::cos(glm::radians(m_Rotation.x));
        const float sin = -glm::sin(glm::radians(m_Rotation.x)); 

        auto movement = m_LateralSpeed * MaxLateralSpeed * deltaTime.GetSeconds();
        m_Position.x += movement.x * cos - movement.y * sin;
        m_Position.y += movement.x * sin + movement.y * cos;
    }
    m_Rotation += m_RotationalSpeed * MaxRotationalSpeed * deltaTime.GetSeconds();
    m_Rotation.y = glm::clamp(m_Rotation.y, -89.0f, 89.0f);

    glm::vec2 col = Algorithms::LineCollisions(glm::vec2(m_Position.x, m_Position.y), walls, Width * 0.5f);
    col += Algorithms::LineCollisions(glm::vec2(m_Position.x, m_Position.y), doors, Width * 0.5f);

    float length = glm::length(col);
    if (length > MaxLateralSpeed) {
        col *= 1.0f / length * MaxLateralSpeed;
    }

    m_Position.x += col.x;
    m_Position.y += col.y;
    m_ViewBob += speed * MaxLateralSpeed * deltaTime.GetSeconds();
    constexpr float bobSpeed = 6.0f, bobAmplitude = 1.0f / 48.0f;
    m_Position.z = 0.5f + speed * bobAmplitude * glm::sin(bobSpeed * m_ViewBob);
}

void Player::UseItem(Core::Timestep deltaTime) {
    RC_ASSERT(m_HeldItemIndex < m_Inventory.size());
    auto& heldItem = m_Inventory[m_HeldItemIndex];

    const float relativeDeltaTime = deltaTime / heldItem.UseDuration;
    m_AnimationProgress += relativeDeltaTime;

    std::visit([&](auto& data) {
        using T = std::decay_t<decltype(data)>;
        glm::vec3 direction(cos(glm::radians(m_Rotation.x)), -sin(glm::radians(m_Rotation.x)), 0.0f);
        if constexpr (std::is_same_v<T, MeleeWeaponData>) {
            if (m_AnimationProgress >= data.AttackTiming && m_AnimationProgress - relativeDeltaTime < data.AttackTiming) {
                m_Areas.emplace_back(m_Position, m_Position + data.AttackLength * direction);
                m_Attacks.emplace_back(std::span{m_Areas.end() - 1, 1 }, data.AttackThickness, data.Damage);
            }
        }
        else if constexpr (std::is_same_v<T, RangedWeaponData>) {
            if (m_AnimationProgress >= data.AttackTiming && m_AnimationProgress - relativeDeltaTime < data.AttackTiming) {
                m_Projectiles.emplace_back(data.Type, m_Position + 0.5f * Width * direction, data.ProjectileSpeed * direction);
                heldItem.Count--;
            }
        } }, heldItem.AdditionalData);

    if (m_AnimationProgress >= 1.0f) {
        m_AnimationProgress = -std::numeric_limits<float>::infinity();

        if (heldItem.Count == 0) {
            m_HeldItemIndex -= m_HeldItemIndex > 0;
        }
    }
}