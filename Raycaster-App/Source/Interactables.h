#pragma once

#include "Entities.h"

#include "Core.h"
#include "Core/Debug/Debug.h"

#include <span>
#include <optional>

namespace InteractableType {
    enum Enumeration : uint8_t {
        Light = 0,
        Barrel,
        ENUMERATION_MAX = Barrel,
    };
}

struct Interactable {
    glm::vec2 Position{};

    uint32_t AtlasIndex = 0;
    InteractableType::Enumeration Type;

    glm::vec3 Scale() const;
};

struct InteractionResult {
    enum class Type {
        None = 0,
        Debug,
    };
    Type Type;
    std::string_view Data;
};

class Interactables {
public:
    void Init();
    void Shutdown();

    void Add(InteractableType::Enumeration type, glm::vec2 position);
    void Remove(size_t index);

    std::optional<InteractableType::Enumeration> CanInteract(const Player& player);
    InteractionResult Interact(const Player& player);

    void UpdateRender(std::span<Sprite> sprites, std::span<Core::Model> models);

    inline size_t Count() const { return m_Interactables.size(); }

    inline const std::span<const Interactable> Get() const { return m_Interactables; }
    inline const Interactable& operator[](size_t index) {
        RC_ASSERT(index < Count());
        return m_Interactables[index];
    }
private:
    std::vector<Interactable> m_Interactables;

    glm::vec2 m_CachedPosition{};
    float m_CachedAngle{};
    size_t m_CachedIndex{};

    static constexpr float s_InteractionRadius = 1.0f;
};