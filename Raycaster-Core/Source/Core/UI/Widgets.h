#pragma once

#include "Types.h"

#include <string>
#include <functional>
#include <optional>

namespace Core::UI::Widgets {
    class CustomRenderWidget : public Widget {
    public:
        CustomRenderWidget(std::function<bool(Surface&)> renderFuntion)
            : m_RenderFunction(renderFuntion) {
        }

        void Update(Surface& current) override {}
        bool Render(Surface& current) override { return m_RenderFunction(current); }
    private:
        std::function<bool(Surface&)> m_RenderFunction;
    };

    class AtlasTextureWidget : public Widget {
    public:
        AtlasTextureWidget(const glm::uvec3& atlasIndices, glm::vec2 atlasScale)
            : m_AtlasIndices(atlasIndices), m_Scale(atlasScale) {}

        void Update(Surface& current) override {}
        bool Render(Surface& current) override;
    private:
        glm::uvec3 m_AtlasIndices;
        glm::vec2 m_Scale;
    };

    template <typename T>
    class TextWidget : public Widget {
    public:
        TextWidget(std::basic_string_view<T> text) : m_Text(text) {}
        TextWidget(std::basic_string_view<T> text, float scale) : m_Text(text), m_Scale(scale) {}

        void Update(Surface& current) override;
        bool Render(Surface& current) override;

    private:
        std::basic_string_view<T> m_Text;
        float m_Scale = 0.0f;
    };

    template <typename T>
    class TextDisplayWidget : public Widget {
    public:
        TextDisplayWidget(std::basic_string_view<T> text, float textScale) : Text(text), TextScale(textScale) {}
        TextDisplayWidget(std::basic_string_view<T> text, float textScale, TextAlignment alignment) : Text(text), TextScale(textScale), Align(alignment) {}

        void Update(Surface& current) override;
        bool Render(Surface& current) override;

        std::basic_string_view<T> Text{};
        
        size_t ColourIndex = 0;

        float TextScale = 1.0f;
        TextAlignment Align = TextAlignment::Left;
    };
    
    template <typename T>
    class TextInputWidget : public Widget {
    public:
        TextInputWidget(std::vector<T>& text, size_t& selectionStart, size_t& selectionEnd, const std::array<glm::vec4, 3>& highlightColours)
            : m_Text(text), m_SelectionStart(selectionStart), m_SelectionEnd(selectionEnd), m_HighlightColours(highlightColours) {}

        void Update(Surface& current) override;
        bool Render(Surface& current) override;
    private:
        std::vector<T>& m_Text;
        
        size_t& m_SelectionStart;
        size_t& m_SelectionEnd;
        
        const std::array<glm::vec4, 3> m_HighlightColours;

        glm::vec2 m_CaretSize{};
        float m_CaretPosition = 0.0f;
    };
    
    template <typename T>
    class TextureTextInputWidget : public Widget {
    public:
        TextureTextInputWidget(std::vector<T>& text, size_t& selectionStart, size_t& selectionEnd, const glm::vec2 boxScale, const glm::uvec3 boxAtlasIndices, const glm::vec2 caretScale, const glm::vec2 selectionMiddleScale, const glm::vec2 selectionEndsScale, const glm::uvec3 selectionAtlasIndices)
            : m_Text(text), m_SelectionStart(selectionStart), m_SelectionEnd(selectionEnd), m_BoxScale(boxScale), m_BoxAtlasIndices(boxAtlasIndices), m_CaretAspectRatio(caretScale.x / caretScale.y), m_SelectionMiddleScale(selectionMiddleScale), m_SelectionEndsScale(selectionEndsScale), m_SelectionAtlasIndices(selectionAtlasIndices) {}

        void Update(Surface& current) override;
        bool Render(Surface& current) override;
    private:
        std::vector<T>& m_Text;

        size_t& m_SelectionStart;
        size_t& m_SelectionEnd;

        glm::vec2 m_CaretSize{};
        float m_CaretPosition = 0.0f;
        float m_CaretAspectRatio;

        const glm::vec2 m_BoxScale;
        const glm::uvec3 m_BoxAtlasIndices;

        const glm::vec2 m_SelectionMiddleScale;
        const glm::vec2 m_SelectionEndsScale;
        const glm::uvec3 m_SelectionAtlasIndices;
    };

    template <typename ValueType, typename CharType>
    class NumericInputWidget : public Widget {
    public:
        NumericInputWidget(ValueType& value, std::vector<CharType>& text, std::function<ValueType(std::basic_string<CharType>)> fromString, std::function<std::basic_string<CharType>(ValueType)> toString, std::function<std::optional<ValueType>(ValueType)> validate, size_t textInputID)
            : m_Value(value), m_Text(text), m_ValueTypeFromString(fromString), m_ValueTypeToString(toString), m_ValidateValue(validate), m_TextInputID(textInputID) {}

        void Update(Surface& current) override;
        bool Render(Surface& current) override { return true; }
    private:
        std::function<ValueType(std::basic_string<CharType>)> m_ValueTypeFromString;
        std::function<std::basic_string<CharType>(ValueType)> m_ValueTypeToString;
        std::function<std::optional<ValueType>(ValueType)> m_ValidateValue;

        std::vector<CharType>& m_Text;

        ValueType& m_Value;

        size_t m_TextInputID;
    };

    class ToggleWidget : public Widget {
    public:
        ToggleWidget(bool enabled, const std::array<glm::vec4, 3>& colours) : m_Enabled(enabled), m_Colours(colours) {}

        void Update(Surface& current) override {}
        bool Render(Surface& current) override;
    private:
        const std::array<glm::vec4, 3> m_Colours;
        bool m_Enabled;
    };

    class AtlasTextureToggleWidget : public Widget {
    public:
        AtlasTextureToggleWidget(bool enabled, const glm::uvec3& boxAtlasIndices, const glm::uvec3& checkAtlasIndices, const glm::vec2& atlasScale)
            : m_Enabled(enabled), m_BoxAtlasIndices(boxAtlasIndices), m_CheckAtlasIndices(checkAtlasIndices), m_Scale(atlasScale) {}

        void Update(Surface& current) override {}
        bool Render(Surface& current) override;

    private:
        bool m_Enabled;

        glm::uvec3 m_BoxAtlasIndices;
        glm::uvec3 m_CheckAtlasIndices;
        glm::vec2 m_Scale;
    };

    class ScrollBarWidget : public Widget {
    public:
        ScrollBarWidget(float& offset, bool moveDown, bool moveUp)
            : m_ScrollOffset(offset), moveDirection(moveUp - moveDown) {
        }

        void Update(Surface& current) override;
        bool Render(Surface& current) override { return true; }
    private:
        float& m_ScrollOffset;

        int32_t moveDirection;
    };

    class AtlasTextureScrollBarWidget : public Widget {
    public:
        AtlasTextureScrollBarWidget(float& offset, bool moveDown, bool moveUp, const glm::uvec3& atlasIndices, glm::vec2 atlasScale)
            : m_ScrollOffset(offset), moveDirection(moveUp - moveDown), m_AtlasIndices(atlasIndices), m_Scale(atlasScale) {
        }

        void Update(Surface& current) override;
        bool Render(Surface& current) override;
    private:
        float& m_ScrollOffset;

        int32_t moveDirection;

        glm::uvec3 m_AtlasIndices;
        glm::vec2 m_Scale;
    };

    template <typename T>
    class SliderWidget : public Widget {
    public:
        SliderWidget(T& value, T min, T max, bool vertical, float sliderSize, const std::array<glm::vec4, 3>& sliderColours)
            : m_Value(value), m_Min(min), m_Max(max), m_SliderDimension(vertical), m_SliderSize(sliderSize), m_SliderColours(sliderColours) { }

        void Update(Surface& current) override;
        bool Render(Surface& current) override;
    private:
        T& m_Value;
        T m_Min;
        T m_Max;

        uint32_t m_SliderDimension;
        float m_SliderSize;

        const std::array<glm::vec4, 3> m_SliderColours;

        friend void ScrollBarWidget::Update(Surface&);
    };

    template <typename T>
    class AtlasTextureSliderWidget : public Widget {
    public:
        AtlasTextureSliderWidget(T& value, T min, T max, bool vertical, glm::vec2 boxAtlasScale, const glm::uvec3& boxAtlasIndices, glm::vec2 sliderSize, glm::vec2 sliderAtlasScale, const glm::uvec3& sliderAtlasIndices)
            : m_Value(value), m_Min(min), m_Max(max), m_SliderDimension(vertical), m_BoxScale(boxAtlasScale), m_BoxAtlasIndices(boxAtlasIndices), m_SliderSize(sliderSize), m_SliderScale(sliderAtlasScale), m_SliderAtlasIndices(sliderAtlasIndices) {}

        void Update(Surface& current) override;
        bool Render(Surface& current) override;
    private:
        T& m_Value;
        T m_Min;
        T m_Max;

        uint32_t m_SliderDimension;

        const glm::vec2 m_BoxScale;
        const glm::uvec3 m_BoxAtlasIndices;
        
        const glm::vec2 m_SliderSize;
        const glm::vec2 m_SliderScale;
        const glm::uvec3 m_SliderAtlasIndices;

        friend void AtlasTextureScrollBarWidget::Update(Surface&);
    };

    class ScrollWidget : public Widget {
    public:
        ScrollWidget(float& offset, bool vertical = true, float speed = 1.0f)
            : m_ScrollOffset(offset), m_ScrollDimension(vertical), m_ScrollSpeed(speed) {}

        void Update(Surface& current) override;
        bool Render(Surface& current) override;
    private:
        float& m_ScrollOffset;

        uint32_t m_ScrollDimension;
        float m_ScrollSpeed;
        float m_ScrollSize = 0.0f;

        friend void ScrollBarWidget::Update(Surface&);
        friend void AtlasTextureScrollBarWidget::Update(Surface&);
        friend void TextInputWidget<char>::Update(Surface&);
        friend void TextInputWidget<wchar_t>::Update(Surface&);
        friend void TextureTextInputWidget<char>::Update(Surface&);
        friend void TextureTextInputWidget<wchar_t>::Update(Surface&);
    };

    class HoverWidget : public Widget {
    public:
        HoverWidget(size_t previousHoverID) : m_PreviousHoverID(previousHoverID) {}

        void Update(Surface& current) override;
        bool Render(Surface& current) override { return false; }
    private:
        size_t m_PreviousHoverID;
    };
}
