#pragma once

#include "Types.h"

#include <string>

namespace Core::UI::Widgets {
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

        void Update(Surface& current) override;
        bool Render(Surface& current) override;

    private:
        std::basic_string_view<T> m_Text;
        float m_Scale = 0.0f;
    };

    class ToggleWidget : public Widget {
    public:
        ToggleWidget(bool enabled) : m_Enabled(enabled) {}

        void Update(Surface& current) override {}
        bool Render(Surface& current) override;

    private:
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

    template <typename T>
    class SliderWidget : public Widget {
    public:
        SliderWidget(T& value, T min, T max, bool vertical, float sliderSize, const std::array<glm::vec4, 3>& sliderColours)
            : m_Value(value), m_Min(min), m_Max(max), m_SliderDimension(vertical), m_SliderSize(sliderSize), m_SliderColours(sliderColours) { }

        void Update(Surface& current) override;
        bool Render(Surface& current) override;
    private:
        T& m_Value;
        const T m_Min;
        const T m_Max;

        size_t m_SliderDimension;
        float m_SliderSize;

        const std::array<glm::vec4, 3> m_SliderColours;
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
        const T m_Min;
        const T m_Max;

        size_t m_SliderDimension;

        const glm::vec2 m_BoxScale;
        const glm::uvec3 m_BoxAtlasIndices;
        
        const glm::vec2 m_SliderSize;
        const glm::vec2 m_SliderScale;
        const glm::uvec3 m_SliderAtlasIndices;
    };

    class ScrollWidget : public Widget {
    public:
        ScrollWidget(float& offset, bool vertical = true, float speed = 1.0f)
            : m_ScrollOffset(offset), m_ScrollDimension(vertical), m_ScrollSpeed(speed) {}

        void Update(Surface& current) override;
        bool Render(Surface& current) override;
    private:
        float& m_ScrollOffset;

        size_t m_ScrollDimension;
        float m_ScrollSpeed;
    };
}
