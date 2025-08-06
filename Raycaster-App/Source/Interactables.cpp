#include "Interactables.h"

#include "Easings.h"

#include <array>

static constexpr InteractionResult DebugInteraction(Interactable& interactable, size_t index) {
    return InteractionResult::Create<InteractionResult::Type::Debug>("You interacted with an interactable", index);
}

static constexpr InteractionResult PickupInteraction(Interactable& interactable, size_t index) {
    return InteractionResult::Create<InteractionResult::Type::Pickup>(Item(0.5f, 1, Animations::AttackDagger, 0.75f, MeleeWeaponData{ 0.75f, 0.25f, 1.0f, 0.75f }), index);
}

static constexpr InteractionResult AnimationInteraction(Interactable& interactable, size_t index);

using InteractionPtr = InteractionResult(*)(Interactable& interactable, size_t);

enum class PlacementType {
    Centre = 0,
    Floor,
    Ceiling,
    Falling,
};

struct InteractableParameters {
    InteractionPtr Interaction = nullptr;
    float Scale;
    AtlasAnimation Animation{};
    PlacementType Placement = PlacementType::Floor;
};

static constinit std::array<InteractableParameters, InteractableType::ENUMERATION_MAX + 1> s_InteractableParameters = []{
    std::array<InteractableParameters, InteractableType::ENUMERATION_MAX + 1> parameters;
    parameters[InteractableType::Light] = InteractableParameters{
        .Scale = 0.5f,
        .Animation = {TextureIndices::Light},
        .Placement = PlacementType::Ceiling,
    };
    parameters[InteractableType::Barrel] = InteractableParameters{
        .Interaction = DebugInteraction,
        .Scale = 0.5f,
        .Animation = {TextureIndices::Barrel},
        .Placement = PlacementType::Floor
    };
    parameters[InteractableType::Chest] = InteractableParameters{
        .Interaction = AnimationInteraction,
        .Scale = 0.5f,
        .Animation = Animations::ChestOpen,
        .Placement = PlacementType::Floor
    };
    parameters[InteractableType::Dagger] = InteractableParameters{
        .Interaction = PickupInteraction,
        .Scale = 0.5f,
        .Animation = {TextureIndices::Floor_Item_Dagger},
        .Placement = PlacementType::Falling
    };
    return parameters;
    }();

static constexpr uint32_t GetAtlasIndex(InteractableType::Enumeration type, Core::Timestep animationProgress) {
    if (type > InteractableType::ENUMERATION_MAX) {
        return 0;
    }

    return s_InteractableParameters[type].Animation.GetFrame(animationProgress);
}

static constexpr InteractionPtr GetInteraction(InteractableType::Enumeration type) {
    if (type > InteractableType::ENUMERATION_MAX) {
        return nullptr;
    }

    return s_InteractableParameters[type].Interaction;
}

static constexpr float GetScale(InteractableType::Enumeration type) {
    if (type > InteractableType::ENUMERATION_MAX) {
        return 1.0f;
    }

    return s_InteractableParameters[type].Scale;
}

static constexpr float CalculatePositionZ(InteractableType::Enumeration type) {
    if (type > InteractableType::ENUMERATION_MAX) {
        return 0.5f;
    }

    switch (s_InteractableParameters[type].Placement) {
    case PlacementType::Falling:
        return GetScale(type) * 0.5f;
    case PlacementType::Centre:
        return 0.5f;
    case PlacementType::Floor:
        return GetScale(type) * 0.5f;
    case PlacementType::Ceiling:
        return 1.0f - GetScale(type) * 0.5f;
    }
}

static constexpr InteractionResult AnimationInteraction(Interactable& interactable, size_t index) {
    if (interactable.AnimationProgress <= 0.0f) {
        interactable.AnimationProgress = 1.0f / s_InteractableParameters[interactable.Type].Animation.FrameCount;
    }

    return {};
}

void Interactables::Init() {
    m_CachedPosition = { -1.0f, -1.0f };
    m_CachedIndex = -1;
}

void Interactables::Shutdown() {
    m_Interactables = std::vector<Interactable>();
}

void Interactables::Add(InteractableType::Enumeration type, glm::vec2 position) {
    RC_ASSERT(type <= InteractableType::ENUMERATION_MAX);
    m_Interactables.emplace_back(glm::vec3{ position, CalculatePositionZ(type) }, GetScale(type), -std::numeric_limits<float>::infinity(), GetAtlasIndex(type, 0.0f), type);
    
    if (s_InteractableParameters[type].Placement == PlacementType::Falling) {
        m_Interactables.back().AnimationProgress = 0.0f;
    }
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
        glm::vec2 delta = m_Interactables[i].Position;
        delta -= m_CachedPosition;
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
    
    if (type == InteractableType::Chest && m_Interactables[m_CachedIndex].AnimationProgress <= 0.0f) {
        Add(InteractableType::Dagger, m_Interactables[m_CachedIndex].Position);
        interaction(m_Interactables[m_CachedIndex], m_CachedIndex);
        
        return InteractionResult::Create<InteractionResult::Type::Add>(std::span<Interactable>(m_Interactables.end() - 1, m_Interactables.end()), 0);
    }

    return interaction(m_Interactables[m_CachedIndex], m_CachedIndex);
}

void Interactables::Update(Core::Timestep deltaTime){
    for (size_t i = 0; i < Count(); i++) {
        Interactable& interactable = m_Interactables[i];
        interactable.AnimationProgress += deltaTime;

        float clampedProgress = glm::clamp(interactable.AnimationProgress.GetSeconds(), 0.0f, 1.0f);
        interactable.AtlasIndex = GetAtlasIndex(interactable.Type, clampedProgress);

        if (s_InteractableParameters[interactable.Type].Placement == PlacementType::Falling) {
            interactable.Position.z = glm::mix(0.6f, GetScale(interactable.Type) * 0.5f, Easings::EaseOutBounce(clampedProgress * 1.25f - 0.25f));
            glm::vec3 direction(2.5f - i % 5, 3.0f - i % 6, 0.0f);
            direction = glm::normalize(direction);
            interactable.Position += 0.5f * direction * (1.0f - clampedProgress) * deltaTime.GetSeconds();
        }
    }
}

void Interactables::UpdateRender(std::span<Sprite> sprites, std::span<Core::Model> models) {
    for (size_t i = 0; i < Count(); i++) {
        const Interactable& interactable = m_Interactables[i];
        
        // Update on Raycaster-layer
        sprites[i].Scale = glm::vec3(interactable.Scale);
        sprites[i].AtlasIndex = interactable.AtlasIndex;
        sprites[i].FlipTexture = false;

        sprites[i].Position = interactable.Position;
        sprites[i].WorldPosition = sprites[i].Position;

        //Update on 3D-layer
        glm::vec2 index = glm::vec2((interactable.AtlasIndex) % ATLASWIDTH, (interactable.AtlasIndex) / ATLASWIDTH);
        models[i].Materials.front()->Parameters.back().Value = 0.0f;
        models[i].Materials.front()->Parameters.front().Value = index;
    }
}