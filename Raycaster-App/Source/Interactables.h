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
        Chest,
        Dagger,
        ENUMERATION_MAX = Dagger,
    };
}

struct Interactable {
    glm::vec3 Position{};
    float Scale = 0.0f;

    Core::Timestep AnimationProgress = 0.0f;
    uint32_t AtlasIndex = 0;
    InteractableType::Enumeration Type;
};

class InteractionResult {
public:
    using variant = std::variant<std::nullopt_t, std::string_view, Item>;
    enum class Type {
        None = 0,
        Debug,
        Pickup,
    };
    Type GetType() const { return static_cast<Type>(Data.index()); };

    template<Type type, typename T>
    static constexpr InteractionResult Create(T data, size_t index) {
        static_assert(std::is_convertible<T, std::variant_alternative_t<static_cast<size_t>(type), variant>>::value);
        return InteractionResult(data, index);
    }

    constexpr InteractionResult() : Data(std::nullopt), Index(-1) {}
    variant Data;
    size_t Index;
private:
    template<typename T>
    constexpr InteractionResult(T data, size_t index) : Data(data), Index(index) {}
};

class Interactables {
public:
    void Init();
    void Shutdown();

    void Add(InteractableType::Enumeration type, glm::vec2 position);
    void Remove(size_t index);

    std::optional<InteractableType::Enumeration> CanInteract(const Player& player);
    InteractionResult Interact(const Player& player);

    void Update(Core::Timestep deltaTime);
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