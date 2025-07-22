#include "Interactables.h"

#include <array>

static constexpr InteractionResult DebugInteraction(size_t index) {
    return InteractionResult::Create<InteractionResult::Type::Debug>("You interacted with an interactable", index);
}

using InteractionPtr = InteractionResult(*)(size_t);

enum class PlacementType {
    Centre = 0,
    Floor,
    Ceiling
};

struct InteractableParameters {
    InteractionPtr Interaction = nullptr;
    glm::vec3 Scale{};
    uint32_t AtlasIndex = 0;
    PlacementType Placement = PlacementType::Floor;
};

constinit std::array<InteractableParameters, InteractableType::ENUMERATION_MAX + 1> s_InteractableParameters = []{
    std::array<InteractableParameters, InteractableType::ENUMERATION_MAX + 1> parameters;
    parameters[InteractableType::Light] = InteractableParameters{
        .Scale = {0.5f, 0.5f, 0.5f},
        .AtlasIndex = 10,
        .Placement = PlacementType::Ceiling,
    };
    parameters[InteractableType::Barrel] = InteractableParameters{
        .Interaction = DebugInteraction,
        .Scale = {0.5f, 0.5f, 0.5f},
        .AtlasIndex = 8,
        .Placement = PlacementType::Floor
    };
    return parameters;
    }();

static constexpr uint32_t GetAtlasIndex(InteractableType::Enumeration type) {
    if (type > InteractableType::ENUMERATION_MAX) {
        return 0;
    }

    return s_InteractableParameters[type].AtlasIndex;
}

static constexpr InteractionPtr GetInteraction(InteractableType::Enumeration type) {
    if (type > InteractableType::ENUMERATION_MAX) {
        return nullptr;
    }

    return s_InteractableParameters[type].Interaction;
}

static constexpr glm::vec3 GetScale(InteractableType::Enumeration type) {
    if (type > InteractableType::ENUMERATION_MAX) {
        return glm::vec3(1.0f);
    }

    return s_InteractableParameters[type].Scale;
}

static constexpr float CalculatePositionZ(InteractableType::Enumeration type, float height) {
    if (type > InteractableType::ENUMERATION_MAX) {
        return 0.5f;
    }

    switch (s_InteractableParameters[type].Placement) {
    case PlacementType::Centre:
        return 0.5f;
    case PlacementType::Floor:
        return height * 0.5f;
    case PlacementType::Ceiling:
        return 1.0f - height * 0.5f;
    }
}

void Interactables::Init() {
    m_CachedPosition = { -1.0f, -1.0f };
    m_CachedIndex = -1;
}

void Interactables::Shutdown() {
    m_Interactables = std::vector<Interactable>();
}

void Interactables::Add(InteractableType::Enumeration type, glm::vec2 position) {
    m_Interactables.emplace_back(position, GetAtlasIndex(type), type);
}

void Interactables::Remove(size_t index) {
    if (index < m_Interactables.size()) {
        m_Interactables[index] = m_Interactables.back();
        m_Interactables.pop_back();
    }
}

std::optional<InteractableType::Enumeration> Interactables::CanInteract(const Player& player) {
    glm::vec2 playerPosition = player.Position;
    if (m_CachedPosition == playerPosition && m_CachedAngle == player.Rotation) {
        if (m_CachedIndex >= Count()) {
            return std::nullopt;
        }

        return std::optional<InteractableType::Enumeration>(m_Interactables[m_CachedIndex].Type);
    }

    m_CachedPosition = playerPosition;
    m_CachedAngle = player.Rotation;
    m_CachedIndex = -1;

    glm::vec2 forward{ glm::cos(glm::radians(player.Rotation )), -glm::sin(glm::radians(player.Rotation )) };

    float maxDot = 0.0f;
    for (size_t i = 0; i < m_Interactables.size(); i++) {
        if (!GetInteraction(m_Interactables[i].Type)) {
            continue;
        }
        auto delta = m_Interactables[i].Position - m_CachedPosition;
        float squaredDistance = glm::dot(delta, delta);
        
        // Check squared distance to save sqrt
        if (squaredDistance > (s_InteractionRadius * s_InteractionRadius)) {
            continue;
        }

        // Divide by squared distance, so closer interactables are preferred
        float dot = glm::dot(delta, forward) / (squaredDistance);
        if (dot > maxDot) {
            m_CachedIndex = i;
            maxDot = dot;
        }
    }

    if (m_CachedIndex == -1) {
        return std::nullopt;
    }

    return std::optional<InteractableType::Enumeration>(m_Interactables[m_CachedIndex].Type);
}

InteractionResult Interactables::Interact(const Player& player) {
    auto type = CanInteract(player);
    
    if (!type) {
        return {};
    }
    
    auto interaction = GetInteraction(type.value());
    if (!interaction) {
        return {};
    }
    
    return interaction(m_CachedIndex);
}

void Interactables::UpdateRender(std::span<Sprite> sprites, std::span<Core::Model> models) {
    for (size_t i = 0; i < Count(); i++) {
        const Interactable& interactable = m_Interactables[i];
        
        // Update on Raycaster-layer
        sprites[i].Scale = GetScale(interactable.Type);
        sprites[i].AtlasIndex = interactable.AtlasIndex;
        sprites[i].FlipTexture = false;

        sprites[i].Position.x = interactable.Position.x;
        sprites[i].Position.y = interactable.Position.y;
        sprites[i].Position.z = CalculatePositionZ(interactable.Type, sprites[i].Scale.z);
        sprites[i].WorldPosition = sprites[i].Position;

        //Update on 3D-layer
        glm::vec2 index = glm::vec2((interactable.AtlasIndex) % ATLASWIDTH, (interactable.AtlasIndex) / ATLASWIDTH);
        models[i].Materials.front()->Parameters.back().Value = 0.0f;
        models[i].Materials.front()->Parameters.front().Value = index;
    }
}

glm::vec3 Interactable::Scale() const {
    return GetScale(Type);
}