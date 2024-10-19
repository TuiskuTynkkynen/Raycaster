#pragma once

#include "Types.h"

namespace Core::UI {
    void Init();
    void Shutdown();

    void Begin(glm::uvec2 screenPosition, glm::uvec2 screenSize, LayoutType layout = LayoutType::Vertical);
    void Update();
    void Render();
    void End();

    void BeginContainer(PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& colour = DefaultColours[0], LayoutType layout = LayoutType::Vertical);
    inline void BeginContainer(glm::vec2 relativeSize, const glm::vec4& colour = DefaultColours[0], LayoutType layout = LayoutType::Vertical) { return BeginContainer(PositioningType::Auto, glm::vec2(1.0f), relativeSize, colour, layout); }
    void EndContainer();

    bool Button(PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultColours[0], const glm::vec4& hoverColour = DefaultColours[1], const glm::vec4& activeColour = DefaultColours[2]);
    inline bool Button(glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultColours[0], const glm::vec4& hoverColour = DefaultColours[1], const glm::vec4& activeColour = DefaultColours[2]) { return Button(PositioningType::Auto, glm::vec2(1.0f), relativeSize, primaryColour, hoverColour, activeColour); }
}

