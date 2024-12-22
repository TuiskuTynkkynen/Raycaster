#include "Widgets.h"

#include "Internal.h"
#include "Core/Renderer/Renderer2D.h"
#include "Core/Debug/Debug.h"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>

constexpr float SCROLLSTEP = 0.125f;

namespace Core::UI::Widgets {
    bool AtlasTextureWidget::Render(Surface& current) {
        RC_ASSERT(Internal::TextureAtlas, "Tried to render UI element with texture before UI setting TextureAtlas")

        uint32_t index = &Internal::System->Elements[Internal::System->ActiveID] == &current ? 2 : &Internal::System->Elements[Internal::System->HoverID] == &current ? 1 : 0;

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), { current.Position.x, current.Position.y, 0.0f });
        transform = glm::scale(transform, { current.Size.x, current.Size.y, 0.0f });

        glm::vec2 atlasOffset(m_AtlasIndices[index] % (uint32_t)Internal::AtlasSize.x, m_AtlasIndices[index] / (uint32_t)Internal::AtlasSize.x * -1.0f);
        
        glm::mat3 texTransform = glm::translate(glm::mat3(1.0f), atlasOffset / Internal::AtlasSize);
        texTransform = glm::scale(texTransform, glm::vec2(m_Scale.x, -m_Scale.y) / Internal::AtlasSize);
        
        Renderer2D::DrawShapeQuad(3, current.Colours[index], transform, texTransform);

        return true;
    }
    
    template <typename T>
    void TextWidget<T>::Update(Surface& current) {
        glm::vec2 size(0.0f);
        size.y = Internal::Font->GetGlyphInfo(' ').Size.y;
        
        if (m_Scale) {
            m_Scale *= current.Size.y / size.y;
        }

        uint32_t lineCount = 0;
        float lineWidth = 0.0f;
        for (size_t i = 0; i < m_Text.length(); i++) {
            if(i == m_Text.length() - 1 || m_Text[i + 1] == '\n') {
                lineWidth += Internal::Font->GetGlyphInfo(m_Text[i]).Size.x;
                size.x = glm::max(size.x, lineWidth);
                lineWidth = 0.0f;

                lineCount++;
                i++;
                continue;
            }

            lineWidth += Internal::Font->GetGlyphInfo(m_Text[i]).Advance;
        }

        if (!m_Scale) {
            glm::vec2 scale(size.x, size.y * lineCount);
            scale = current.Size / scale;
            m_Scale = std::min(scale.x, scale.y);
        }

        size *= m_Scale * glm::vec2(-0.5f, 0.75f - lineCount * 0.5f);
        current.Position += size;
    }

    template <typename T>
    bool TextWidget<T>::Render(Surface& current) {
        uint32_t colourIndex = UI::Internal::System->ActiveID == current.ParentID ? 2 : UI::Internal::System->HoverID == current.ParentID ? 1 : 0;

        Renderer2D::DrawShapeString(m_Text, current.Position.x, current.Position.y, m_Scale, current.Colours[colourIndex], true);
        
        return true;
    }

    template TextWidget<char>;
    template TextWidget<wchar_t>;

    bool ToggleWidget::Render(Surface& current) {
        constexpr glm::vec3 AxisZ(0.0f, 0.0f, 1.0f);

        uint32_t index = &Internal::System->Elements[Internal::System->ActiveID] == &current ? 2 : &Internal::System->Elements[Internal::System->HoverID] == &current ? 1 : 0;
        glm::vec4& colour = current.Colours[index];

        Renderer2D::DrawFlatRoundedQuadEdge(current.Size, 0.125f, 0.2f, 5, { current.Position.x, current.Position.y, 0.0f }, glm::vec3(1.0f), colour);

        if (m_Enabled) {
            Renderer2D::DrawRotatedFlatRoundedQuad({ current.Size.x * 0.2f, current.Size.y * 0.8f }, 0.5f, 5, { current.Position.x, current.Position.y, 0.0f }, 45.0f, AxisZ, glm::vec3(1.0f), { colour.r * 1.2f, colour.g * 1.2f, colour.b * 1.2f, colour.a });
            Renderer2D::DrawRotatedFlatRoundedQuad({ current.Size.x * 0.2f, current.Size.y * 0.8f }, 0.5f, 5, { current.Position.x, current.Position.y, 0.0f }, -45.0f, AxisZ, glm::vec3(1.0f), { colour.r * 1.2f, colour.g * 1.2f, colour.b * 1.2f, colour.a });
        }

        return true;
    }

    bool AtlasTextureToggleWidget::Render(Surface& current) {
        constexpr glm::vec3 AxisZ(0.0f, 0.0f, 1.0f);

        uint32_t index = &Internal::System->Elements[Internal::System->ActiveID] == &current ? 2 : &Internal::System->Elements[Internal::System->HoverID] == &current ? 1 : 0;
        glm::vec4& colour = current.Colours[index];

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), { current.Position.x, current.Position.y, 0.0f });
        transform = glm::scale(transform, { current.Size.x, current.Size.y, 0.0f });

        glm::vec2 atlasOffset(m_BoxAtlasIndices[index] % (uint32_t)Internal::AtlasSize.x, m_BoxAtlasIndices[index] / (uint32_t)Internal::AtlasSize.x * -1.0f);

        glm::mat3 texTransform = glm::translate(glm::mat3(1.0f), atlasOffset / Internal::AtlasSize);
        texTransform = glm::scale(texTransform, glm::vec2(m_Scale.x, -m_Scale.y) / Internal::AtlasSize);

        Renderer2D::DrawShapeQuad(3, current.Colours[index], transform, texTransform);

        if (m_Enabled) {
            atlasOffset = glm::vec2(m_CheckAtlasIndices[index] % (uint32_t)Internal::AtlasSize.x, m_CheckAtlasIndices[index] / (uint32_t)Internal::AtlasSize.x * -1.0f);

            texTransform = glm::translate(glm::mat3(1.0f), atlasOffset / Internal::AtlasSize);
            texTransform = glm::scale(texTransform, glm::vec2(m_Scale.x, -m_Scale.y) / Internal::AtlasSize);

            Renderer2D::DrawShapeQuad(3, current.Colours[index], transform, texTransform);
        }

        return true;
    }

    template <typename T>
    void SliderWidget<T>::Update(Surface& current) {
        if (&Internal::System->Elements[Internal::System->ActiveID] != &current) {
            return;
        }

        float maxPosition = (1.0f - m_SliderSize) - 0.125f * glm::min(glm::abs(current.Size.x), glm::abs(current.Size.y));
        if (std::is_integral<T>::value) {
            m_Value = glm::clamp<T>(glm::round((Internal::Input->MouseState.Position[m_SliderDimension] - current.Position[m_SliderDimension] + current.Size[m_SliderDimension] * 0.5f * maxPosition) / (current.Size[m_SliderDimension] * maxPosition) * (m_Max - m_Min)), m_Min, m_Max);
        } else {
            m_Value = glm::clamp<T>((Internal::Input->MouseState.Position[m_SliderDimension] - current.Position[m_SliderDimension] + current.Size[m_SliderDimension] * 0.5f * maxPosition) / (current.Size[m_SliderDimension] * maxPosition) * (m_Max - m_Min), m_Min, m_Max);
        }
    }

    template <typename T>
    bool SliderWidget<T>::Render(Surface& current) {
        uint32_t index = &Internal::System->Elements[Internal::System->ActiveID] == &current ? 2 : &Internal::System->Elements[Internal::System->HoverID] == &current ? 1 : 0;
        const glm::vec4& colour = current.Colours[index];

        Renderer2D::DrawFlatRoundedQuad(current.Size, 0.2f, 2, { current.Position.x, current.Position.y, 0.0f }, glm::vec3(1.0f), colour * 0.8f);

        glm::vec3 sliderPosition(0.0f);
        float maxPosition = current.Size[m_SliderDimension] * (1.0f - m_SliderSize) - 0.125f * glm::min(glm::abs(current.Size.x), glm::abs(current.Size.y));
        sliderPosition[m_SliderDimension] = current.Position[m_SliderDimension] + maxPosition * glm::clamp<float>((float)m_Value / (m_Max - m_Min) - 0.5, -0.5f, 0.5f);
        sliderPosition[1 - m_SliderDimension] = current.Position[1 - m_SliderDimension];
        
        glm::vec3 sliderSize(0.0f);
        sliderSize[m_SliderDimension] = current.Size[m_SliderDimension] * m_SliderSize;
        sliderSize[1 - m_SliderDimension] = current.Size[1 - m_SliderDimension] * 0.85f;
        
        Renderer2D::DrawFlatShapeQuad(sliderPosition, sliderSize, m_SliderColours[index]);
        
        Renderer2D::DrawFlatRoundedQuadEdge(current.Size, 0.075f, 0.2f, 5, { current.Position.x, current.Position.y, 0.0f }, glm::vec3(1.0f), { colour.r * 1.2f, colour.g * 1.2f, colour.b * 1.2f, colour.a });

        return true;
    }

    template SliderWidget<int8_t>;
    template SliderWidget<uint8_t>;
    template SliderWidget<int32_t>;
    template SliderWidget<uint32_t>;
    template SliderWidget<int64_t>;
    template SliderWidget<uint64_t>;
    template SliderWidget<float>;
    template SliderWidget<double>;

    template <typename T>
    void AtlasTextureSliderWidget<T>::Update(Surface& current) {
        if (&Internal::System->Elements[Internal::System->ActiveID] != &current) {
            return;
        }


        if (std::is_integral<T>::value) {
            m_Value = glm::clamp<T>(glm::round((Internal::Input->MouseState.Position[m_SliderDimension] - current.Position[m_SliderDimension] + current.Size[m_SliderDimension] * 0.5f * (1.0f - m_SliderSize[m_SliderDimension])) / (current.Size[m_SliderDimension] * (1.0f - m_SliderSize[m_SliderDimension])) * (m_Max - m_Min)), m_Min, m_Max);
        }
        else {
            m_Value = glm::clamp<T>((Internal::Input->MouseState.Position[m_SliderDimension] - current.Position[m_SliderDimension] + current.Size[m_SliderDimension] * 0.5f * (1.0f - m_SliderSize[m_SliderDimension])) / (current.Size[m_SliderDimension] * (1.0f - m_SliderSize[m_SliderDimension])) * (m_Max - m_Min), m_Min, m_Max);
        }
    }

    template <typename T>
    bool AtlasTextureSliderWidget<T>::Render(Surface& current) {
        uint32_t index = &Internal::System->Elements[Internal::System->ActiveID] == &current ? 2 : &Internal::System->Elements[Internal::System->HoverID] == &current ? 1 : 0;
        glm::vec4& colour = current.Colours[index];

        //Box
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), { current.Position.x, current.Position.y, 0.0f });
        transform = glm::scale(transform, { current.Size.x, current.Size.y, 0.0f });

        glm::vec2 atlasOffset(m_BoxAtlasIndices[index] % (uint32_t)Internal::AtlasSize.x, m_BoxAtlasIndices[index] / (uint32_t)Internal::AtlasSize.x * -1.0f);

        glm::mat3 texTransform = glm::translate(glm::mat3(1.0f), atlasOffset / Internal::AtlasSize);
        texTransform = glm::scale(texTransform, glm::vec2(m_BoxScale.x, -m_BoxScale.y) / Internal::AtlasSize);

        Renderer2D::DrawShapeQuad(3, current.Colours[index], transform, texTransform);

        //Slider
        glm::vec3 sliderPosition(0.0f);
        sliderPosition[m_SliderDimension] = current.Position[m_SliderDimension] + (current.Size[m_SliderDimension] * (1.0f - m_SliderSize[m_SliderDimension])) * glm::clamp<float>((float)m_Value / (m_Max - m_Min) - 0.5, -0.5f, 0.5f);
        sliderPosition[1 - m_SliderDimension] = current.Position[1 - m_SliderDimension];

        transform = glm::translate(glm::mat4(1.0f), sliderPosition);
        transform = glm::scale(transform, { m_SliderSize.x * current.Size.x, m_SliderSize.y * current.Size.y, 0.0f });
    
        atlasOffset = glm::vec2(m_SliderAtlasIndices[index] % (uint32_t)Internal::AtlasSize.x, m_SliderAtlasIndices[index] / (uint32_t)Internal::AtlasSize.x * -1.0f);
        texTransform = glm::translate(glm::mat3(1.0f), atlasOffset / Internal::AtlasSize);
        texTransform = glm::scale(texTransform, glm::vec2(m_SliderScale.x, -m_SliderScale.y) / Internal::AtlasSize);

        Renderer2D::DrawShapeQuad(3, current.Colours[index], transform, texTransform);
        
        return true;
    }

    template AtlasTextureSliderWidget<int8_t>;
    template AtlasTextureSliderWidget<uint8_t>;
    template AtlasTextureSliderWidget<int32_t>;
    template AtlasTextureSliderWidget<uint32_t>;
    template AtlasTextureSliderWidget<int64_t>;
    template AtlasTextureSliderWidget<uint64_t>;
    template AtlasTextureSliderWidget<float>;
    template AtlasTextureSliderWidget<double>;
    
    void ScrollWidget::Update(Surface& current) {
        size_t parentIndex = current.ParentID;
        size_t currentIndex = parentIndex + 1;
        
        //Get the index of the current element
        for (; currentIndex && currentIndex < UI::Internal::System->Elements.size(); currentIndex = UI::Internal::System->Elements[currentIndex].SiblingID) {
            if (&UI::Internal::System->Elements[currentIndex] == &current) {
                break;
            }
        }
        
        //Update the positions of the current element's children
        uint32_t childCount = 0;
        for (size_t i = currentIndex + 1; i < UI::Internal::System->Elements.size() && UI::Internal::System->Elements[i].ParentID == currentIndex; i = UI::Internal::System->Elements[i].SiblingID) {
            Surface& child = UI::Internal::System->Elements[i];
            
            if (child.Positioning <= PositioningType::Offset) {
                child.Positioning = PositioningType::Offset;
                child.Position[m_ScrollDimension] -= m_ScrollOffset * m_ScrollSpeed;

                m_ScrollSize += child.Size[m_ScrollDimension];
                childCount++;
            }
        }

        m_ScrollSize += 0.025f * current.Size[m_ScrollDimension] * (childCount + 2) - current.Size[m_ScrollDimension];
        m_ScrollSize = glm::abs(m_ScrollSize / (current.Size[m_ScrollDimension] * m_ScrollSpeed));

        if (currentIndex == Internal::System->HoverID && Internal::Input->MouseState.ScrollOffset) {
            m_ScrollOffset -= SCROLLSTEP * Internal::Input->MouseState.ScrollOffset;
            m_ScrollOffset = glm::clamp(m_ScrollOffset, 0.0f, m_ScrollSize);
        }
    }

    bool ScrollWidget::Render(Surface& current) {
        uint32_t index = &Internal::System->Elements[Internal::System->ActiveID] == &current ? 2 : &Internal::System->Elements[Internal::System->HoverID] == &current ? 1 : 0;
        const glm::vec4& colour = current.Colours[index];

        
        if (current.Size.x * current.Size.y != 0.0f || colour.a != 0.0f) { 
            Renderer2D::DrawFlatShapeQuad({ current.Position.x, current.Position.y, 0.0f }, { current.Size.x, current.Size.y, 0.0f }, colour);
        }

        return true;
    }

    void ScrollBarWidget::Update(Surface& current) {
        //Get the index of the current element
        size_t currentIndex = current.ParentID + 1;
        for (; currentIndex; currentIndex = UI::Internal::System->Elements[currentIndex].SiblingID) {
            if (currentIndex >= UI::Internal::System->Elements.size()) {
                currentIndex = 0;
            }

            if (&UI::Internal::System->Elements[currentIndex] == &current) {
                break;
            }
        }

        if(!currentIndex || current.ChildCount != 3 || currentIndex + 3 >= UI::Internal::System->Elements.size()){
            RC_WARN("UI element with ScrollBarWidget should have three children");
            return;
        }

        //Set the slider max value to scrollSize
        Surface& parent = Internal::System->Elements[current.ParentID];
        if (parent.Widget && typeid(*parent.Widget) == typeid(ScrollWidget)) {
            float scrollSize = ((ScrollWidget*)parent.Widget.get())->m_ScrollSize;
            
            if (moveDirection != 0) {
                m_ScrollOffset += moveDirection * SCROLLSTEP;
                m_ScrollOffset = glm::clamp(m_ScrollOffset, 0.0f, scrollSize);
            }

            Surface& child = Internal::System->Elements[currentIndex + 1];
            if(child.Widget) {
                if (typeid(*child.Widget) == typeid(SliderWidget<float>)) {
                    ((SliderWidget<float>*)child.Widget.get())->m_Max = scrollSize;
                    
                    size_t scrollDimension = ((ScrollWidget*)parent.Widget.get())->m_ScrollDimension;
                    ((SliderWidget<float>*)child.Widget.get())->m_SliderSize = glm::clamp(glm::abs(parent.Size[scrollDimension] / scrollSize), 0.05f, 0.9f);
                } else {
                    RC_WARN("The first child of UI element with ScrollBarWidget should have a SliderWidget member");
                }
            }
        }
    }

    void AtlasTextureScrollBarWidget::Update(Surface& current) {
        //Get the index of the current element
        size_t currentIndex = current.ParentID + 1;
        for (; currentIndex; currentIndex = UI::Internal::System->Elements[currentIndex].SiblingID) {
            if (currentIndex >= UI::Internal::System->Elements.size()) {
                currentIndex = 0;
            }

            if (&UI::Internal::System->Elements[currentIndex] == &current) {
                break;
            }
        }

        if(!currentIndex || current.ChildCount != 3 || currentIndex + 3 >= UI::Internal::System->Elements.size()){
            RC_WARN("UI element with ScrollBarWidget should have three children");
            return;
        }

        //Set the slider max value to scrollSize
        Surface& parent = Internal::System->Elements[current.ParentID];
        if (parent.Widget && typeid(*parent.Widget) == typeid(ScrollWidget)) {
            float scrollSize = ((ScrollWidget*)parent.Widget.get())->m_ScrollSize;
            
            if (moveDirection != 0) {
                m_ScrollOffset += moveDirection * SCROLLSTEP;
                m_ScrollOffset = glm::clamp(m_ScrollOffset, 0.0f, scrollSize);
            }

            Surface& child = Internal::System->Elements[currentIndex + 1];
            if(child.Widget) {
                if (typeid(*child.Widget) == typeid(AtlasTextureSliderWidget<float>)) {
                    ((AtlasTextureSliderWidget<float>*)child.Widget.get())->m_Max = scrollSize;
                } else {
                    RC_WARN("The first child of UI element with AtlasTextureScrollBarWidget should have a AtlasTextureSliderWidget member");
                }
            }
        }
    }

    bool AtlasTextureScrollBarWidget::Render(Surface& current) {
        RC_ASSERT(Internal::TextureAtlas, "Tried to render UI TextureScrollBar element before UI setting TextureAtlas")
    
        size_t parentIndex = current.ParentID;
        size_t currentIndex = parentIndex + 1;

        //Get the index of the current element
        for (; currentIndex && currentIndex < UI::Internal::System->Elements.size(); currentIndex = UI::Internal::System->Elements[currentIndex].SiblingID) {
            if (&UI::Internal::System->Elements[currentIndex] == &current) {
                break;
            }
        }

        //Update the positions of the current element's children
        bool hovered = false;
        for (size_t i = currentIndex + 1; !hovered && i < UI::Internal::System->Elements.size() && UI::Internal::System->Elements[i].ParentID == currentIndex; i = UI::Internal::System->Elements[i].SiblingID) {
            hovered |= Internal::System->HoverID == i;
            hovered |= Internal::System->ActiveID == i;
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), { current.Position.x, current.Position.y, 0.0f });
        transform = glm::scale(transform, { current.Size.x, current.Size.y, 0.0f });

        glm::vec2 atlasOffset(m_AtlasIndices[(size_t)hovered] % (uint32_t)Internal::AtlasSize.x, m_AtlasIndices[(size_t)hovered] / (uint32_t)Internal::AtlasSize.x * -1.0f);

        glm::mat3 texTransform = glm::translate(glm::mat3(1.0f), atlasOffset / Internal::AtlasSize);
        texTransform = glm::scale(texTransform, glm::vec2(m_Scale.x, -m_Scale.y) / Internal::AtlasSize);

        Renderer2D::DrawShapeQuad(3, current.Colours[(size_t)hovered], transform, texTransform);

        return true;
    }
}