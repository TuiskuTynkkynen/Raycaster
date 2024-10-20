#pragma once

#include "Types.h"
#include "Core/Font/Font.h"

#include <memory>
#include <string>

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

    template <typename T>
    bool Button(std::basic_string_view<T> text, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& buttonColours = DefaultColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours);
    inline bool Button(std::string_view text, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& buttonColours = DefaultColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours) { return Button<char>(text, positioning, position, relativeSize, buttonColours, textColours); }
    inline bool Button(std::wstring_view text, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& buttonColours = DefaultColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours) { return Button<wchar_t>(text, positioning, position, relativeSize, buttonColours, textColours); }
    inline bool Button(std::string_view text, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& buttonColours = DefaultColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours) { return Button<char>(text, PositioningType::Auto, glm::vec2(1.0f), relativeSize, buttonColours, textColours); }
    inline bool Button(std::wstring_view text, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& buttonColours = DefaultColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours) { return Button<wchar_t>(text, PositioningType::Auto, glm::vec2(1.0f), relativeSize, buttonColours, textColours); }

    bool Button(PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultColours[0], const glm::vec4& hoverColour = DefaultColours[1], const glm::vec4& activeColour = DefaultColours[2]);
    inline bool Button(glm::vec2 relativeSize, const glm::vec4 & primaryColour = DefaultColours[0], const glm::vec4 & hoverColour = DefaultColours[1], const glm::vec4 & activeColour = DefaultColours[2]) { return Button(PositioningType::Auto, glm::vec2(1.0f), relativeSize, primaryColour, hoverColour, activeColour); }

    void Text(std::string_view text, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultTextColours[0], const glm::vec4& hoverColour = DefaultTextColours[1], const glm::vec4& activeColour = DefaultTextColours[2]);
    void Text(std::wstring_view text, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultTextColours[0], const glm::vec4& hoverColour = DefaultTextColours[1], const glm::vec4& activeColour = DefaultTextColours[2]);
    inline void Text(std::string_view text, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultTextColours[0], const glm::vec4& hoverColour = DefaultTextColours[1], const glm::vec4& activeColour = DefaultTextColours[2]) { Text(text, PositioningType::Auto, glm::vec2(0.0f), relativeSize, primaryColour, hoverColour, activeColour); }
    inline void Text(std::wstring_view text, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultTextColours[0], const glm::vec4& hoverColour = DefaultTextColours[1], const glm::vec4& activeColour = DefaultTextColours[2]) { Text(text, PositioningType::Auto, glm::vec2(0.0f), relativeSize, primaryColour, hoverColour, activeColour); }

    void SetFont(std::shared_ptr<Core::Font> font);
}

