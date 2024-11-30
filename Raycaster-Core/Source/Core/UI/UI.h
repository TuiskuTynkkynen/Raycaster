#pragma once

#include "Types.h"
#include "Core/Font/Font.h"
#include "Core/Events/MouseEvent.h"

#include <memory>
#include <string>

namespace Core::UI {
    void Init();
    void Shutdown();

    void Begin(glm::uvec2 screenPosition, glm::uvec2 screenSize, LayoutType layout = LayoutType::Vertical, const glm::vec4& colour = DefaultBackgroundColours[0]);
    void Update();
    void Render();
    void End();

    void BeginContainer(PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& colour = DefaultBackgroundColours[0], LayoutType layout = LayoutType::Vertical);
    inline void BeginContainer(glm::vec2 relativeSize, const glm::vec4& colour = DefaultColours[0], LayoutType layout = LayoutType::Vertical) { return BeginContainer(PositioningType::Auto, glm::vec2(1.0f), relativeSize, colour, layout); }
    void EndContainer();

    void BeginScrollContainer(float& offset, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, bool vertical = true, float speed = 1.0f, const glm::vec4& primaryColour = DefaultBackgroundColours[0], const glm::vec4& hoverColour = DefaultBackgroundColours[1]);
    inline void BeginScrollContainer(float& offset, glm::vec2 relativeSize, bool vertical = true, float speed = 1.0f, const glm::vec4& primaryColour = DefaultBackgroundColours[0], const glm::vec4& hoverColour = DefaultBackgroundColours[1]) { BeginScrollContainer(offset, PositioningType::Auto, glm::vec2(1.0f), relativeSize, vertical, speed, primaryColour, hoverColour); }
    inline void EndScrollContainer() { EndContainer(); }

    template <typename T>
    bool Button(std::basic_string_view<T> text, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& buttonColours = DefaultColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours);
    inline bool Button(std::string_view text, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& buttonColours = DefaultColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours) { return Button<char>(text, positioning, position, relativeSize, buttonColours, textColours); }
    inline bool Button(std::wstring_view text, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& buttonColours = DefaultColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours) { return Button<wchar_t>(text, positioning, position, relativeSize, buttonColours, textColours); }
    inline bool Button(std::string_view text, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& buttonColours = DefaultColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours) { return Button<char>(text, PositioningType::Auto, glm::vec2(1.0f), relativeSize, buttonColours, textColours); }
    inline bool Button(std::wstring_view text, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& buttonColours = DefaultColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours) { return Button<wchar_t>(text, PositioningType::Auto, glm::vec2(1.0f), relativeSize, buttonColours, textColours); }

    bool Button(PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultColours[0], const glm::vec4& hoverColour = DefaultColours[1], const glm::vec4& activeColour = DefaultColours[2]);
    inline bool Button(glm::vec2 relativeSize, const glm::vec4 & primaryColour = DefaultColours[0], const glm::vec4 & hoverColour = DefaultColours[1], const glm::vec4 & activeColour = DefaultColours[2]) { return Button(PositioningType::Auto, glm::vec2(1.0f), relativeSize, primaryColour, hoverColour, activeColour); }

    template <typename T>
    bool TextureButton(std::basic_string_view<T> text, glm::uvec3 atlasIndices, glm::vec2 atlasSize, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& colours = DefaultTextureColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours);
    inline bool TextureButton(std::string_view text, glm::uvec3 atlasIndices, glm::vec2 atlasSize, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& buttonColours = DefaultTextureColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours) { return TextureButton<char>(text, atlasIndices, atlasSize, positioning, position, relativeSize, buttonColours, textColours); }
    inline bool TextureButton(std::wstring_view text, glm::uvec3 atlasIndices, glm::vec2 atlasSize, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& buttonColours = DefaultTextureColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours) { return TextureButton<wchar_t>(text, atlasIndices, atlasSize, positioning, position, relativeSize, buttonColours, textColours); }
    inline bool TextureButton(std::string_view text, glm::uvec3 atlasIndices, glm::vec2 atlasSize, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& buttonColours = DefaultTextureColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours) { return TextureButton<char>(text, atlasIndices, atlasSize, PositioningType::Auto, glm::vec2(1.0f), relativeSize, buttonColours, textColours); }
    inline bool TextureButton(std::wstring_view text, glm::uvec3 atlasIndices, glm::vec2 atlasSize, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& buttonColours = DefaultTextureColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours) { return TextureButton<wchar_t>(text, atlasIndices, atlasSize, PositioningType::Auto, glm::vec2(1.0f), relativeSize, buttonColours, textColours); }

    bool TextureButton(glm::uvec3 atlasIndices, glm::vec2 atlasSize, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& primaryColour = glm::vec4(1.0f), const glm::vec4& hoverColour = glm::vec4(1.0f), const glm::vec4& activeColour = glm::vec4(1.0f));
    inline bool TextureButton(glm::uvec3 atlasIndices, glm::vec2 atlasSize, glm::vec2 relativeSize, const glm::vec4& primaryColour = glm::vec4(1.0f), const glm::vec4& hoverColour = glm::vec4(1.0f), const glm::vec4& activeColour = glm::vec4(1.0f)) { return TextureButton(atlasIndices, atlasSize, PositioningType::Auto, glm::vec2(1.0f), relativeSize, primaryColour, hoverColour, activeColour); }

    void Texture(glm::uvec3 atlasIndices, glm::vec2 atlasSize, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& colour = glm::vec4(1.0f));
    inline void Texture(glm::uvec3 atlasIndices, glm::vec2 atlasSize, glm::vec2 relativeSize, const glm::vec4& colour = glm::vec4(1.0f)) { Texture(atlasIndices, atlasSize, PositioningType::Auto, glm::vec2(1.0f), relativeSize, colour); }

    void Text(std::string_view text, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultTextColours[0], const glm::vec4& hoverColour = DefaultTextColours[1], const glm::vec4& activeColour = DefaultTextColours[2]);
    void Text(std::wstring_view text, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultTextColours[0], const glm::vec4& hoverColour = DefaultTextColours[1], const glm::vec4& activeColour = DefaultTextColours[2]);
    inline void Text(std::string_view text, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultTextColours[0], const glm::vec4& hoverColour = DefaultTextColours[1], const glm::vec4& activeColour = DefaultTextColours[2]) { Text(text, PositioningType::Auto, glm::vec2(0.0f), relativeSize, primaryColour, hoverColour, activeColour); }
    inline void Text(std::wstring_view text, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultTextColours[0], const glm::vec4& hoverColour = DefaultTextColours[1], const glm::vec4& activeColour = DefaultTextColours[2]) { Text(text, PositioningType::Auto, glm::vec2(0.0f), relativeSize, primaryColour, hoverColour, activeColour); }

    void Toggle(bool& enabled, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultColours[0], const glm::vec4& hoverColour = DefaultColours[1], const glm::vec4& activeColour = DefaultColours[2]);
    inline void Toggle(bool& enabled, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultColours[0], const glm::vec4& hoverColour = DefaultColours[1], const glm::vec4& activeColour = DefaultColours[2]) { Toggle(enabled, PositioningType::Auto, glm::vec2(1.0f), relativeSize, primaryColour, hoverColour, activeColour); }
    
    void TextureToggle(bool& enabled, const glm::uvec3& boxAtlasIndices, const glm::uvec3& checkAtlasIndices, glm::vec2 atlasScale, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultTextureColours[0], const glm::vec4& hoverColour = DefaultTextureColours[1], const glm::vec4& activeColour = DefaultTextureColours[2]);
    inline void TextureToggle(bool& enabled, const glm::uvec3& boxAtlasIndices, const glm::uvec3& checkAtlasIndices, glm::vec2 atlasScale, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultTextureColours[0], const glm::vec4& hoverColour = DefaultTextureColours[1], const glm::vec4& activeColour = DefaultTextureColours[2]) { TextureToggle(enabled, boxAtlasIndices, checkAtlasIndices, atlasScale, PositioningType::Auto, glm::vec2(1.0f), relativeSize, primaryColour, hoverColour, activeColour); }

    template <typename T>
    void Slider(T& value, T min, T max, bool vertical, float sliderSize, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& sliderColours = DefaultHighlightColours, const std::array<glm::vec4, 3>& boxColours = DefaultColours);
    template <typename T>
    void Slider(T& value, T min, T max, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& sliderColours = DefaultHighlightColours, const std::array<glm::vec4, 3>& boxColours = DefaultColours) { Slider<T>(value, min, max, false, 0.1f, positioning, position, relativeSize, sliderColours, boxColours); }
    template <typename T>
    void Slider(T& value, T min, T max, bool vertical, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& sliderColours = DefaultHighlightColours, const std::array<glm::vec4, 3>& boxColours = DefaultColours) { Slider<T>(value, min, max, vertical, 0.1f, PositioningType::Auto, glm::vec2(1.0f), relativeSize, sliderColours, boxColours); }
    template <typename T>
    void Slider(T& value, T min, T max, float sliderSize, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& sliderColours = DefaultHighlightColours, const std::array<glm::vec4, 3>& boxColours = DefaultColours) { Slider<T>(value, min, max, false, sliderSize, PositioningType::Auto, glm::vec2(1.0f), relativeSize, sliderColours, boxColours); }
    template <typename T>
    void Slider(T& value, T min, T max, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& sliderColours = DefaultHighlightColours, const std::array<glm::vec4, 3>& boxColours = DefaultColours) { Slider<T>(value, min, max, false, 0.1f, PositioningType::Auto, glm::vec2(1.0f), relativeSize, sliderColours, boxColours); }
    
    template <typename T>
    void TextureSlider(T& value, T min, T max, bool vertical, const glm::uvec3& boxAtlasIndices, glm::vec2 boxAtlasScale, glm::vec2 sliderSize, const glm::uvec3& sliderAtlasIndices, glm::vec2 sliderAtlasScale, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& boxColours = DefaultTextureColours);
    template <typename T>
    void TextureSlider(T& value, T min, T max, const glm::uvec3& boxAtlasIndices, glm::vec2 boxAtlasScale, glm::vec2 sliderSize, const glm::uvec3& sliderAtlasIndices, glm::vec2 sliderAtlasScale, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& boxColours = DefaultTextureColours) { TextureSlider<T>(value, min, max, false, boxAtlasIndices, boxAtlasScale, sliderSize, sliderAtlasIndices, sliderAtlasScale, positioning, position, relativeSize, boxColours); }
    template <typename T>
    void TextureSlider(T& value, T min, T max, bool vertical, const glm::uvec3& boxAtlasIndices, glm::vec2 boxAtlasScale, glm::vec2 sliderSize, const glm::uvec3& sliderAtlasIndices, glm::vec2 sliderAtlasScale, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& boxColours = DefaultTextureColours) { TextureSlider<T>(value, min, max, vertical, boxAtlasIndices, boxAtlasScale, sliderSize, sliderAtlasIndices, sliderAtlasScale, PositioningType::Auto, glm::vec2(1.0f), relativeSize, boxColours); }
    template <typename T>
    void TextureSlider(T& value, T min, T max, const glm::uvec3& boxAtlasIndices, glm::vec2 boxAtlasScale, glm::vec2 sliderSize, const glm::uvec3& sliderAtlasIndices, glm::vec2 sliderAtlasScale, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& boxColours = DefaultTextureColours) { TextureSlider<T>(value, min, max, false, boxAtlasIndices, boxAtlasScale, sliderSize, sliderAtlasIndices, sliderAtlasScale, PositioningType::Auto, glm::vec2(1.0f), relativeSize, boxColours); }
   
    void ScrollBar(float& offset, float buttonSize, glm::vec2 position, glm::vec2 relativeSize, std::array<glm::vec4, 3>& baseColours = DefaultColours, std::array<glm::vec4, 3>& highlightColours = DefaultHighlightColours);
    inline void ScrollBar(float& offset, float buttonSize, glm::vec2 relativeSize, std::array<glm::vec4, 3>& baseColours = DefaultColours, std::array<glm::vec4, 3>& highlightColours = DefaultHighlightColours) { ScrollBar(offset, buttonSize, glm::vec2(0.5f * (1.0f - relativeSize.x), 0.0f), relativeSize, baseColours, highlightColours); }
    inline void ScrollBar(float& offset, glm::vec2 relativeSize, std::array<glm::vec4, 3>& baseColours = DefaultColours, std::array<glm::vec4, 3>& highlightColours = DefaultHighlightColours) { ScrollBar(offset, 0.1f, glm::vec2(0.5f * (1.0f - relativeSize.x), 0.0f), relativeSize, baseColours, highlightColours); }

    void SetFont(std::shared_ptr<Core::Font> font);
    void SetTextureAtlas(std::shared_ptr<Core::Texture2D> atlas, glm::uvec2 atlasSize);

    bool OnMouseMovedEvent(MouseMoved& event);
    bool OnMouseButtonPressedEvent(MouseButtonPressed& event);
    bool OnMouseButtonReleasedEvent(MouseButtonReleased& event);
    bool OnMouseScrollEvent(MouseScrolled& event);
}