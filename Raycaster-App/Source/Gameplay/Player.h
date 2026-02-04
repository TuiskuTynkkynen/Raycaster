#pragma once

#include "Map.h"
#include "Entities.h"
#include "Renderables.h"
#include "Projectiles.h"
#include "InputCode.h"

#include "Core/Base/Timestep.h"

#include <span>
#include <vector>

class Player {
public:
    static constexpr float Width = 0.8f;
    static constexpr float MaxHealth = 10.0f;

    void Init(const Map& map);
    void Shutdown();

    void Update(std::span<const LineCollider> walls, std::span<const LineCollider> doors, Core::Timestep deltaTime);
    void UpdateRender(Renderables& renderables);
    
    bool DamageAreas(std::span<const LineCollider> areas, float areaThickness, float damage);
    void PickUp(Item item);

    inline const glm::vec3 GetPosition() const { return m_Position; }
    inline const float GetYaw() const { return m_Rotation.x; }
    inline const float GetPitch() const { return m_Rotation.y; }
    inline const glm::vec3 GetScale() const { return m_Scale; }
    
    inline const float GetHealth() const { return m_Health; }

    inline const Item& GetHeldItem() const {
        RC_ASSERT(m_HeldItemIndex < m_Inventory.size());
        return m_Inventory[m_HeldItemIndex];
    }
    inline float GetAnimationProgress() const { return m_AnimationProgress; }

    inline const std::span<const Attack> GetAttacks() const { return m_Attacks; };
    inline const std::span<const Projectile> GetProjectiles() const { return m_Projectiles; };

    inline bool ShouldInteract() const { return m_ShouldInteract; }

    bool OnInputPressed(Settings::KeyBinds::InputCode input, bool repeated);
    bool OnInputReleased(Settings::KeyBinds::InputCode input);

    bool OnMouseEvent(Core::MouseMoved event);
    bool OnMouseEvent(Core::MouseScrolled event);
    
    void OnEvent(Core::Event& event);
private:
    void Move(std::span<const LineCollider> walls, std::span<const LineCollider> doors, Core::Timestep deltaTime);
    void UseItem(Core::Timestep deltaTime);
    bool SwitchItem(size_t index);

    glm::vec3 m_Position{};
    glm::vec3 m_Scale{};
    glm::vec2 m_Rotation{};
    float m_ViewBob = 0.0f;

    glm::vec2 m_LateralSpeed{};
    glm::vec2 m_RotationalSpeed{};
    
    glm::vec2 m_MousePosition;
    float m_MouseScrollCooldown = 0.0f;

    float m_Health = MaxHealth;

    Core::Timestep m_AnimationProgress = -std::numeric_limits<float>::infinity();
    size_t m_HeldItemIndex = 0;
    std::array<Item, 3> m_Inventory;

    std::vector<LineCollider> m_Areas;
    std::vector<Attack> m_Attacks;
    std::vector<Projectile> m_Projectiles;

    bool m_ShouldInteract = false;
};

