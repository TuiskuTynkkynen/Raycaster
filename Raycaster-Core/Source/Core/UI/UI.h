#pragma once

#include "Types.h"
#include "Core/Base/Timestep.h"
#include "Core/Font/Font.h"
#include "Core/Events/MouseEvent.h"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/TextEvent.h"

#include <memory>
#include <string>

namespace Core::UI {
    void Init();
    void Shutdown();

    void Begin(glm::uvec2 screenPosition, glm::uvec2 screenSize, LayoutType layout = LayoutType::Vertical, const glm::vec4& colour = DefaultBackgroundColours[0]);
    void Update(Timestep deltaTime);
    void Render();
    void End(Timestep deltaTime);

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
    bool TextureButton(std::basic_string_view<T> text, const AtlasProperties& atlasProperties, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& colours = DefaultTextureColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours);
    inline bool TextureButton(std::string_view text, const AtlasProperties& atlasProperties, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& buttonColours = DefaultTextureColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours) { return TextureButton<char>(text, atlasProperties, positioning, position, relativeSize, buttonColours, textColours); }
    inline bool TextureButton(std::wstring_view text, const AtlasProperties& atlasProperties, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& buttonColours = DefaultTextureColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours) { return TextureButton<wchar_t>(text, atlasProperties, positioning, position, relativeSize, buttonColours, textColours); }
    inline bool TextureButton(std::string_view text, const AtlasProperties& atlasProperties, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& buttonColours = DefaultTextureColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours) { return TextureButton<char>(text, atlasProperties, PositioningType::Auto, glm::vec2(1.0f), relativeSize, buttonColours, textColours); }
    inline bool TextureButton(std::wstring_view text, const AtlasProperties& atlasProperties, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& buttonColours = DefaultTextureColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours) { return TextureButton<wchar_t>(text, atlasProperties, PositioningType::Auto, glm::vec2(1.0f), relativeSize, buttonColours, textColours); }

    bool TextureButton(const AtlasProperties& atlasProperties, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& primaryColour = glm::vec4(1.0f), const glm::vec4& hoverColour = glm::vec4(1.0f), const glm::vec4& activeColour = glm::vec4(1.0f));
    inline bool TextureButton(const AtlasProperties& atlasProperties, glm::vec2 relativeSize, const glm::vec4& primaryColour = glm::vec4(1.0f), const glm::vec4& hoverColour = glm::vec4(1.0f), const glm::vec4& activeColour = glm::vec4(1.0f)) { return TextureButton(atlasProperties, PositioningType::Auto, glm::vec2(1.0f), relativeSize, primaryColour, hoverColour, activeColour); }

    void Texture(const AtlasProperties& atlasProperties, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& colour = glm::vec4(1.0f));
    inline void Texture(const AtlasProperties& atlasProperties, glm::vec2 relativeSize, const glm::vec4& colour = glm::vec4(1.0f)) { Texture(atlasProperties, PositioningType::Auto, glm::vec2(1.0f), relativeSize, colour); }

    void Text(std::string_view text, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultTextColours[0], const glm::vec4& hoverColour = DefaultTextColours[1], const glm::vec4& activeColour = DefaultTextColours[2]);
    void Text(std::wstring_view text, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultTextColours[0], const glm::vec4& hoverColour = DefaultTextColours[1], const glm::vec4& activeColour = DefaultTextColours[2]);
    inline void Text(std::string_view text, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultTextColours[0], const glm::vec4& hoverColour = DefaultTextColours[1], const glm::vec4& activeColour = DefaultTextColours[2]) { Text(text, PositioningType::Auto, glm::vec2(0.0f), relativeSize, primaryColour, hoverColour, activeColour); }
    inline void Text(std::wstring_view text, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultTextColours[0], const glm::vec4& hoverColour = DefaultTextColours[1], const glm::vec4& activeColour = DefaultTextColours[2]) { Text(text, PositioningType::Auto, glm::vec2(0.0f), relativeSize, primaryColour, hoverColour, activeColour); }
    
    void TextInputField(std::vector<char>& text, std::string_view label, float textScale, float& scrollOffset, size_t& selectionStart, size_t& selectionEnd, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& boxColours = DefaultColours, const std::array<glm::vec4, 3>& highlightColours = DefaultHighlightColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours, const std::array<glm::vec4, 3>& highlightedTextColours = DefaultColours);
    inline void TextInputField(std::vector<char>& text, std::string_view label, float textScale, float& scrollOffset, size_t& selectionStart, size_t& selectionEnd, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& boxColours = DefaultColours, const std::array<glm::vec4, 3>& highlightColours = DefaultHighlightColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours, const std::array<glm::vec4, 3>& highlightedTextColours = DefaultColours) { TextInputField(text, label, textScale, scrollOffset, selectionStart, selectionEnd, PositioningType::Auto, glm::vec2(0.0f), relativeSize, boxColours, highlightColours, textColours, highlightedTextColours); }
    inline void TextInputField(std::vector<char>& text, std::string_view label, float textScale, float& scrollOffset, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& boxColours = DefaultColours, const std::array<glm::vec4, 3>& highlightColours = DefaultHighlightColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours, const std::array<glm::vec4, 3>& highlightedTextColours = DefaultColours) { size_t sentinel = -1; TextInputField(text, label, textScale, scrollOffset, sentinel, sentinel, PositioningType::Auto, glm::vec2(0.0f), relativeSize, boxColours, highlightColours, textColours, highlightedTextColours); }
    inline void TextInputField(std::vector<char>& text, std::string_view label, float& scrollOffset, size_t& selectionStart, size_t& selectionEnd, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& boxColours = DefaultColours, const std::array<glm::vec4, 3>& highlightColours = DefaultHighlightColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours, const std::array<glm::vec4, 3>& highlightedTextColours = DefaultColours) { TextInputField(text, label, 1.0f, scrollOffset, selectionStart, selectionEnd, PositioningType::Auto, glm::vec2(0.0f), relativeSize, boxColours, highlightColours, textColours, highlightedTextColours); }
    inline void TextInputField(std::vector<char>& text, std::string_view label, float& scrollOffset, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& boxColours = DefaultColours, const std::array<glm::vec4, 3>& highlightColours = DefaultHighlightColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours, const std::array<glm::vec4, 3>& highlightedTextColours = DefaultColours) { size_t sentinel = -1; TextInputField(text, label, 1.0f, scrollOffset, sentinel, sentinel, PositioningType::Auto, glm::vec2(0.0f), relativeSize, boxColours, highlightColours, textColours, highlightedTextColours); }
    void TextInputField(std::vector<wchar_t>& text, std::wstring_view label, float textScale, float& scrollOffset, size_t& selectionStart, size_t& selectionEnd, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& boxColours = DefaultColours, const std::array<glm::vec4, 3>& highlightColours = DefaultHighlightColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours, const std::array<glm::vec4, 3>& highlightedTextColours = DefaultColours);
    inline void TextInputField(std::vector<wchar_t>& text, std::wstring_view label, float textScale, float& scrollOffset, size_t& selectionStart, size_t& selectionEnd, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& boxColours = DefaultColours, const std::array<glm::vec4, 3>& highlightColours = DefaultHighlightColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours, const std::array<glm::vec4, 3>& highlightedTextColours = DefaultColours) { TextInputField(text, label, textScale, scrollOffset, selectionStart, selectionEnd, PositioningType::Auto, glm::vec2(0.0f), relativeSize, boxColours, highlightColours, textColours, highlightedTextColours);  }
    inline void TextInputField(std::vector<wchar_t>& text, std::wstring_view label, float textScale, float& scrollOffset, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& boxColours = DefaultColours, const std::array<glm::vec4, 3>& highlightColours = DefaultHighlightColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours, const std::array<glm::vec4, 3>& highlightedTextColours = DefaultColours) { size_t sentinel = -1; TextInputField(text, label, textScale, scrollOffset, sentinel, sentinel, PositioningType::Auto, glm::vec2(0.0f), relativeSize, boxColours, highlightColours, textColours, highlightedTextColours);  }
    inline void TextInputField(std::vector<wchar_t>& text, std::wstring_view label, float& scrollOffset, size_t& selectionStart, size_t& selectionEnd, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& boxColours = DefaultColours, const std::array<glm::vec4, 3>& highlightColours = DefaultHighlightColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours, const std::array<glm::vec4, 3>& highlightedTextColours = DefaultColours) { TextInputField(text, label, 1.0f, scrollOffset, selectionStart, selectionEnd, PositioningType::Auto, glm::vec2(0.0f), relativeSize, boxColours, highlightColours, textColours, highlightedTextColours);  }
    inline void TextInputField(std::vector<wchar_t>& text, std::wstring_view label, float& scrollOffset, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& boxColours = DefaultColours, const std::array<glm::vec4, 3>& highlightColours = DefaultHighlightColours, const std::array<glm::vec4, 3>& textColours = DefaultTextColours, const std::array<glm::vec4, 3>& highlightedTextColours = DefaultColours) { size_t sentinel = -1; TextInputField(text, label, 1.0f, scrollOffset, sentinel, sentinel, PositioningType::Auto, glm::vec2(0.0f), relativeSize, boxColours, highlightColours, textColours, highlightedTextColours); }

    void Toggle(bool& enabled, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultColours[0], const glm::vec4& hoverColour = DefaultColours[1], const glm::vec4& activeColour = DefaultColours[2]);
    inline void Toggle(bool& enabled, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultColours[0], const glm::vec4& hoverColour = DefaultColours[1], const glm::vec4& activeColour = DefaultColours[2]) { Toggle(enabled, PositioningType::Auto, glm::vec2(1.0f), relativeSize, primaryColour, hoverColour, activeColour); }
    
    void TextureToggle(bool& enabled, const AtlasProperties& boxAtlasProperties, const glm::uvec3& checkAtlasIndices, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultTextureColours[0], const glm::vec4& hoverColour = DefaultTextureColours[1], const glm::vec4& activeColour = DefaultTextureColours[2]);
    inline void TextureToggle(bool& enabled, const AtlasProperties& boxAtlasProperties, const glm::uvec3& checkAtlasIndices, glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultTextureColours[0], const glm::vec4& hoverColour = DefaultTextureColours[1], const glm::vec4& activeColour = DefaultTextureColours[2]) { TextureToggle(enabled, boxAtlasProperties, checkAtlasIndices, PositioningType::Auto, glm::vec2(1.0f), relativeSize, primaryColour, hoverColour, activeColour); }

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
    void TextureSlider(T& value, T min, T max, bool vertical, const AtlasProperties& boxAtlasProperties, glm::vec2 sliderSize, const AtlasProperties& sliderAtlasProperties, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& boxColours = DefaultTextureColours);
    template <typename T>
    void TextureSlider(T& value, T min, T max, const AtlasProperties& boxAtlasProperties, glm::vec2 sliderSize, const AtlasProperties& sliderAtlasProperties, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& boxColours = DefaultTextureColours) { TextureSlider<T>(value, min, max, false, boxAtlasProperties, sliderSize, sliderAtlasProperties, positioning, position, relativeSize, boxColours); }
    template <typename T>
    void TextureSlider(T& value, T min, T max, bool vertical, const AtlasProperties& boxAtlasProperties, glm::vec2 sliderSize, const AtlasProperties& sliderAtlasProperties, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& boxColours = DefaultTextureColours) { TextureSlider<T>(value, min, max, vertical, boxAtlasProperties, sliderSize, sliderAtlasProperties, PositioningType::Auto, glm::vec2(1.0f), relativeSize, boxColours); }
    template <typename T>
    void TextureSlider(T& value, T min, T max, const AtlasProperties& boxAtlasProperties, glm::vec2 sliderSize, const AtlasProperties& sliderAtlasProperties, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& boxColours = DefaultTextureColours) { TextureSlider<T>(value, min, max, false, boxAtlasProperties, sliderSize, sliderAtlasProperties, PositioningType::Auto, glm::vec2(1.0f), relativeSize, boxColours); }
   
    void ScrollBar(float& offset, float buttonSize, glm::vec2 position, glm::vec2 relativeSize, std::array<glm::vec4, 3>& baseColours = DefaultColours, std::array<glm::vec4, 3>& highlightColours = DefaultHighlightColours);
    inline void ScrollBar(float& offset, float buttonSize, glm::vec2 relativeSize, std::array<glm::vec4, 3>& baseColours = DefaultColours, std::array<glm::vec4, 3>& highlightColours = DefaultHighlightColours) { ScrollBar(offset, buttonSize, glm::vec2(0.5f * (1.0f - relativeSize.x), 0.0f), relativeSize, baseColours, highlightColours); }
    inline void ScrollBar(float& offset, glm::vec2 relativeSize, std::array<glm::vec4, 3>& baseColours = DefaultColours, std::array<glm::vec4, 3>& highlightColours = DefaultHighlightColours) { ScrollBar(offset, 0.1f, glm::vec2(0.5f * (1.0f - relativeSize.x), 0.0f), relativeSize, baseColours, highlightColours); }

    void TextureScrollBar(float& offset, glm::vec2 buttonOffset, glm::vec2 buttonSize, const AtlasProperties& buttonAtlasProperties, glm::vec2 sliderSize, const AtlasProperties& sliderAtlasProperties, const AtlasProperties& baseAtlasProperties, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& colours = DefaultTextureColours);
    inline void TextureScrollBar(float& offset, glm::vec2 buttonOffset, glm::vec2 buttonSize, const AtlasProperties& buttonAtlasProperties, glm::vec2 sliderSize, const AtlasProperties& sliderAtlasProperties, const AtlasProperties& baseAtlasProperties, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& colours = DefaultTextureColours) { TextureScrollBar(offset, buttonOffset, buttonSize, buttonAtlasProperties, sliderSize, sliderAtlasProperties, baseAtlasProperties, glm::vec2(0.5f * (1.0f - relativeSize.x), 0.0f), relativeSize, colours); }
    inline void TextureScrollBar(float& offset, glm::vec2 buttonSize, const AtlasProperties& buttonAtlasProperties, glm::vec2 sliderSize, const AtlasProperties& sliderAtlasProperties, const AtlasProperties& baseAtlasProperties, glm::vec2 position, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& colours = DefaultTextureColours) { TextureScrollBar(offset, glm::vec2(0.0f, 0.5f * (1.0f - buttonSize.y)), buttonSize, buttonAtlasProperties, sliderSize, sliderAtlasProperties, baseAtlasProperties, position, relativeSize, colours); }
    inline void TextureScrollBar(float& offset, glm::vec2 buttonSize, const AtlasProperties& buttonAtlasProperties, glm::vec2 sliderSize, const AtlasProperties& sliderAtlasProperties, const AtlasProperties& baseAtlasProperties, glm::vec2 relativeSize, const std::array<glm::vec4, 3>& colours = DefaultTextureColours) { TextureScrollBar(offset, glm::vec2(0.0f, 0.5f * (1.0f - buttonSize.y)), buttonSize, buttonAtlasProperties, sliderSize, sliderAtlasProperties, baseAtlasProperties, glm::vec2(0.5f * (1.0f - relativeSize.x), 0.0f), relativeSize, colours); }

    void SetFont(std::shared_ptr<Core::Font> font);
    void SetTextureAtlas(std::shared_ptr<Core::Texture2D> atlas, glm::uvec2 atlasSize);

    void OnEvent(Event& event);

    bool OnMouseMovedEvent(MouseMoved& event);
    bool OnMouseButtonPressedEvent(MouseButtonPressed& event);
    bool OnMouseButtonReleasedEvent(MouseButtonReleased& event);
    bool OnMouseScrollEvent(MouseScrolled& event);
    bool OnKeyPressedEvent(KeyPressed& event);
    bool OnTextInputEvent(TextInput& event);
}