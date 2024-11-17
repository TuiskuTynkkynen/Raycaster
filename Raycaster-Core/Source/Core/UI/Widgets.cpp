#include "Widgets.h"

#include "Internal.h"
#include "Core/Renderer/Renderer2D.h"
#include "Core/Debug/Debug.h"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>

namespace Core::UI::Widgets {
    bool AtlasTextureWidget::Render(Surface& current) {
        RC_ASSERT(Internal::TextureAtlas, "Tried to render UI element with texture before UI setting TextureAtlas")

        uint32_t index = &Internal::System->Elements[Internal::System->ActiveID] == &current ? 2 : &Internal::System->Elements[Internal::System->HoverID] == &current ? 1 : 0;

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), { current.Position.x, current.Position.y, 0.0f });
        transform = glm::scale(transform, { current.Size.x, current.Size.y, 0.0f });

        glm::vec2 atlasOffset(m_AtlasIndices[index] % (uint32_t)Internal::AtlasSize.x, m_AtlasIndices[index] / (uint32_t)Internal::AtlasSize.x);
        
        glm::mat3 texTransform = glm::translate(glm::mat3(1.0f), atlasOffset / Internal::AtlasSize);
        texTransform = glm::scale(texTransform, glm::vec2(m_Scale.x, -m_Scale.y) / Internal::AtlasSize);
        
        Renderer2D::DrawShapeQuad(3, current.Colours[index], transform, texTransform);

        return true;
    }

	template <typename T>
	void TextWidget<T>::Update(Surface& current) {
        glm::vec2 size(0.0f);
        size.y = Internal::Font->GetGlyphInfo(' ').Size.y;

        for (size_t i = 0; i < m_Text.length(); i++) {
            if (i == m_Text.length() - 1) {
                size.x += Internal::Font->GetGlyphInfo(m_Text[i]).Size.x;
                break;
            }
            size.x += Internal::Font->GetGlyphInfo(m_Text[i]).Advance;
        }

        glm::vec2 scale = (current.Size) / size;
        m_Scale = std::min(scale.x, scale.y);

        size *= m_Scale * glm::vec2(-0.5f, 0.25f);
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

        glm::vec2 atlasOffset(m_BoxAtlasIndices[index] % (uint32_t)Internal::AtlasSize.x, m_BoxAtlasIndices[index] / (uint32_t)Internal::AtlasSize.x);

        glm::mat3 texTransform = glm::translate(glm::mat3(1.0f), atlasOffset / Internal::AtlasSize);
        texTransform = glm::scale(texTransform, glm::vec2(m_Scale.x, -m_Scale.y) / Internal::AtlasSize);

        Renderer2D::DrawShapeQuad(3, current.Colours[index], transform, texTransform);

        if (m_Enabled) {
            atlasOffset = glm::vec2(m_CheckAtlasIndices[index] % (uint32_t)Internal::AtlasSize.x, m_CheckAtlasIndices[index] / (uint32_t)Internal::AtlasSize.x);

            texTransform = glm::translate(glm::mat3(1.0f), atlasOffset / Internal::AtlasSize);
            texTransform = glm::scale(texTransform, glm::vec2(m_Scale.x, -m_Scale.y) / Internal::AtlasSize);

            Renderer2D::DrawShapeQuad(3, current.Colours[index], transform, texTransform);
        }

        return true;
    }
}