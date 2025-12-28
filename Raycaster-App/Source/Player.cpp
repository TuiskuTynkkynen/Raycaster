#include "Player.h"

#include "Core/Debug/Assert.h"

#include <cmath>
#include <limits>
#include <type_traits>
#include <vector>

void Player::Init(const Map& map) {
    m_Position = glm::vec3((float)map.GetWidth() / 2, (float)map.GetHeight() / 2, 0.5f);
    m_Scale = glm::vec3(Width * 0.5f);
    m_Rotation = 90.0f;

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

    auto& sprite = renderables.GetNextSprite();
    auto& model = renderables.GetNextModel();

    uint32_t atlasIndex = heldItem.UseAnimation.GetFrame(m_AnimationProgress);

    // Update on Raycaster-layer
    float epsilon = 1e-3f;
    sprite.Position = glm::vec3(m_Position.x, m_Position.y, 0.5f - (1.0f - heldItem.Scale) * epsilon);
    sprite.Position.x += glm::cos(glm::radians(m_Rotation)) * epsilon;
    sprite.Position.y += -glm::sin(glm::radians(m_Rotation)) * epsilon;
    sprite.WorldPosition = sprite.Position;

    sprite.Scale = glm::vec3(2.0f * heldItem.Scale * epsilon);
    sprite.AtlasIndex = atlasIndex;
    sprite.FlipTexture = false;

    //Update on 3D-layer
    glm::vec2 index = glm::vec2((atlasIndex) % ATLASWIDTH, (atlasIndex) / ATLASWIDTH);
    model.Materials.front()->Parameters.back().Value = glm::vec2(0.0f, 0.0f);
    model.Materials.front()->Parameters.front().Value = index;
}

bool Player::DamageAreas(std::span<const LineCollider> attack, float thickness, float damage) {
    const bool hit = Algorithms::LineCollisions(m_Position, attack, thickness + Width * 0.5f) != glm::vec2(0.0f);
    m_Health -= damage * hit;

    return hit;
}

void Player::PickUp(Item item) {
    m_Inventory[m_HeldItemIndex] = item;
}

bool Player::OnKeyEvent(Core::KeyPressed event) {
    bool HoldingItem = m_HeldItemIndex < m_Inventory.size() && m_Inventory[m_HeldItemIndex].UseDuration;
    bool UsingItem = HoldingItem && m_AnimationProgress >= 0.0f;

    switch (event.GetKeyCode()) {
    case RC_KEY_W:
        m_LateralSpeed = 1.0f;
        return true;
    case RC_KEY_S:
        m_LateralSpeed = -1.0f;
        return true;
    case RC_KEY_A:
        m_RotationalSpeed = 1.0f;
        return true;
    case RC_KEY_D:
        m_RotationalSpeed = -1.0f;
        return true;
    case RC_KEY_Q:
        if (HoldingItem && !UsingItem) {
            m_AnimationProgress = 0.0f;
        }
        return true;
    case RC_KEY_SPACE:
        m_ShouldInteract = true;
        return true;
    }

    return false;
}


bool Player::OnKeyEvent(Core::KeyReleased event) {
    switch (event.GetKeyCode()) {
    case RC_KEY_W:
        m_LateralSpeed = 0.0f;
        return true;
    case RC_KEY_S:
        m_LateralSpeed = 0.0f;
        return true;
    case RC_KEY_A:
        m_RotationalSpeed = 0.0f;
        return true;
    case RC_KEY_D:
        m_RotationalSpeed = 0.0f;
        return true;
    }

    return false;
}

void Player::Move(std::span<const LineCollider> walls, std::span<const LineCollider> doors, Core::Timestep deltaTime) {
    const float MaxLateralSpeed = 2.0f;
    const float MaxRotationalSpeed = 180.0f;

    glm::vec3 front(0.0f);
    front.x = cos(glm::radians(m_Rotation));
    front.y = -sin(glm::radians(m_Rotation)); //player y is flipped (array index)
    front.z = 0.0f;

    m_Position += front * m_LateralSpeed * MaxLateralSpeed * deltaTime.GetSeconds();
    m_Rotation += m_RotationalSpeed * MaxRotationalSpeed * deltaTime.GetSeconds();

    glm::vec2 col = Algorithms::LineCollisions(glm::vec2(m_Position.x, m_Position.y), walls, Width * 0.5f);
    col += Algorithms::LineCollisions(glm::vec2(m_Position.x, m_Position.y), doors, Width * 0.5f);

    float length = glm::length(col);
    if (length > MaxLateralSpeed) {
        col *= 1.0f / length * MaxLateralSpeed;
    }

    m_Position.x += col.x;
    m_Position.y += col.y;
}

void Player::UseItem(Core::Timestep deltaTime) {
    RC_ASSERT(m_HeldItemIndex < m_Inventory.size());
    auto& heldItem = m_Inventory[m_HeldItemIndex];

    const float relativeDeltaTime = deltaTime / heldItem.UseDuration;
    m_AnimationProgress += relativeDeltaTime;

    std::visit([&](auto& data) {
        using T = std::decay_t<decltype(data)>;
        glm::vec3 direction(cos(glm::radians(m_Rotation)), -sin(glm::radians(m_Rotation)), 0.0f);
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
            heldItem = { .Scale = 0.5f, .Count = 0 };
            m_HeldItemIndex -= m_HeldItemIndex < 0;
        }
    }
}