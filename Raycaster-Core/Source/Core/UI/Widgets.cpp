#include "Widgets.h"

#include "Internal.h"
#include "Core/Renderer/Renderer2D.h"

namespace Core::UI::Widgets {
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
        
        Renderer2D::DrawString(m_Text, current.Position.x, current.Position.y, m_Scale, current.Colours[colourIndex], true);
        
        return true;
    }

    template TextWidget<char>;
    template TextWidget<wchar_t>;
}