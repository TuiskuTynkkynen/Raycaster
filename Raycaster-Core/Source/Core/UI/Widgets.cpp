#include "Widgets.h"

#include "Internal.h"
#include "Core/Base/Clipboard.h"
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
        size.y = static_cast<float>(Internal::Font->GetGlyphInfo(' ').Size.y);
        
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
            m_Scale = glm::min(scale.x, scale.y);
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

    template class TextWidget<char>;
    template class TextWidget<wchar_t>;

    template <typename T>
    void TextDisplayWidget<T>::Update(Surface& current) {
        float lineHeight = static_cast<float>(Internal::Font->GetGlyphInfo(' ').Size.y);
        TextScale *= current.Size.y / lineHeight;
        
        if (Align == TextAlignment::Left) {
            current.Position.x -= current.Size.x * 0.5f;
            return;
        }

        float textWidth = 0.0f, lineWidth = 0.0f;
        for (size_t i = 0; i < Text.length(); i++) {
            if (i == Text.length() - 1 || Text[i + 1] == '\n') {
                textWidth = glm::max(textWidth, lineWidth + Internal::Font->GetGlyphInfo(Text[i]).Size.x);
                lineWidth = 0.0f;

                i++;
                continue;
            }

            lineWidth += Internal::Font->GetGlyphInfo(Text[i]).Advance;
        }

        current.Position.x += 0.5f * current.Size.x - textWidth * TextScale;
    }

    template <typename T>
    bool TextDisplayWidget<T>::Render(Surface& current) {
        Renderer2D::DrawShapeString(Text, current.Position.x, current.Position.y, TextScale, current.Colours[ColourIndex], true);

        return true;
    }

    template class TextDisplayWidget<char>;
    template class TextDisplayWidget<wchar_t>;
    
    template <typename T>
    void TextInputWidget<T>::Update(Surface& current) {
        m_SelectionStart = glm::min(m_SelectionStart, m_Text.size());
        m_SelectionEnd = glm::min(m_SelectionEnd, m_Text.size());
        bool canMouseSelect = true;

        auto updateSelection = [this]() -> bool {
            if (Internal::Input->TestModKey(Internal::Keys::Control) && Internal::Input->TestInputKey(Internal::Keys::A)) {
                m_SelectionStart = m_Text.size();
                m_SelectionEnd = 0;
                return true;
            }

            int64_t direction = Internal::Input->TestInputKey(Internal::Keys::Rigth) - Internal::Input->TestInputKey(Internal::Keys::Left);
            
            bool skipWordSearch = false;
            if (Internal::Input->TestInputKey(Internal::Keys::Home) || Internal::Input->TestInputKey(Internal::Keys::Up) || Internal::Input->TestInputKey(Internal::Keys::PageUp)) {
                direction = -1 * m_SelectionStart;
                skipWordSearch = true;
            }

            if (Internal::Input->TestInputKey(Internal::Keys::End) || Internal::Input->TestInputKey(Internal::Keys::Down) || Internal::Input->TestInputKey(Internal::Keys::PageDown)) {
                direction = m_Text.size() - m_SelectionStart;
                skipWordSearch = true;
            }

            if (!direction) {
                return false;
            }

            if (!Internal::Input->TestModKey(Internal::Keys::Shift) && m_SelectionEnd != m_SelectionStart) {
                size_t caret = direction < 0 ? glm::min(m_SelectionEnd, m_SelectionStart) : glm::max(m_SelectionEnd, m_SelectionStart);
                m_SelectionEnd = m_SelectionStart = caret;
                return true;
            }

            if (!skipWordSearch && Internal::Input->TestModKey(Internal::Keys::Control)) {
                enum class CharacterType : char8_t {
                    None = 0,
                    Space,
                    Alphanumeric,
                    Other,
                };
                
                int64_t offset = 0;
                
                size_t current = (1 + direction) / 2;
                size_t previous = 1 - current;
                std::array<CharacterType, 2> window = { CharacterType::None, CharacterType::None };
                for (size_t i = m_SelectionStart - (direction < 0); direction && i < m_Text.size(); i += direction) {
                    uint32_t c = m_Text[i];

                    window[previous] = window[current];
                    window[current] = c == ' ' ? CharacterType::Space : (c > 0xBF || (c >= 'A' && c <= 'z') || (c >= '0' && c <= '9')) ? CharacterType::Alphanumeric : CharacterType::Other;
                    
                    if (window[0] == CharacterType::Space && window[1] > CharacterType::Space) {
                        break;
                    }

                    if ((char8_t)window[0] + (char8_t)window[1] == (char8_t)CharacterType::Alphanumeric + (char8_t)CharacterType::Other) {
                        break;
                    }

                    offset += direction;
                }

                if (offset) {
                    direction = offset;
                }
            }

            m_SelectionStart = glm::clamp<int64_t>(m_SelectionStart + direction, 0, m_Text.size());
            if (!Internal::Input->TestModKey(Internal::Keys::Shift)) {
                m_SelectionEnd = m_SelectionStart;
            }
            return true;
        };
        auto removeText = [this]() -> bool {
            size_t offset = Internal::Input->TestInputKey(Internal::Keys::Backspace) ? 1 : Internal::Input->TestInputKey(Internal::Keys::Delete) ? 0 : 2;
            bool cut = Internal::Input->TestModKey(Internal::Keys::Control) && Internal::Input->TestInputKey(Internal::Keys::X) && m_SelectionStart != m_SelectionEnd;
            
            if (offset == 2 && !cut) {
                return false;
            }

            if (m_SelectionStart != m_SelectionEnd) {
                size_t min = glm::min(m_SelectionStart, m_SelectionEnd);
                auto selectionEnd = m_Text.begin() + glm::max(m_SelectionStart, m_SelectionEnd);
                
                if (cut) {
                    std::basic_string<T> selection(m_Text.begin() + min, selectionEnd);
                    Clipboard::SetClipboard<T>(selection);
                }

                m_Text.erase(m_Text.begin() + min, selectionEnd);
                m_SelectionStart = m_SelectionEnd = min;
                return true;
            }
                
            size_t eraseAt = m_SelectionStart - offset;
            if (eraseAt < m_Text.size()) {
                m_Text.erase(m_Text.begin() + eraseAt);
                m_SelectionStart = m_SelectionEnd = eraseAt;
                return true;
            }

            return false;
        };
        auto insertText = [this]() -> bool {
            size_t inputSize = Internal::Input->KeyboardState.InputedText.size();
            bool paste = Internal::Input->TestModKey(Internal::Keys::Control) && Internal::Input->TestInputKey(Internal::Keys::V);
            
            if (!inputSize && !paste) {
                return false;
            }

            if (m_SelectionStart != m_SelectionEnd) {
                size_t min = glm::min(m_SelectionStart, m_SelectionEnd);
                m_Text.erase(m_Text.begin() + min, m_Text.begin() + glm::max(m_SelectionStart, m_SelectionEnd));
                m_SelectionStart = m_SelectionEnd = min;
            }

            if (m_Text.size() == m_Text.capacity()) {
                return false;
            }

            for (size_t i = 0; i < inputSize && m_Text.size() < m_Text.capacity(); i++) {
                m_Text.emplace(m_Text.begin() + m_SelectionStart++, Internal::Input->KeyboardState.InputedText[i]);
            }

            if (!inputSize) {
                std::basic_string<T> clipboard = Clipboard::GetClipboard<T>();

                for (size_t i = 0; i < clipboard.size() && m_Text.size() < m_Text.capacity(); i++) {
                    m_Text.emplace(m_Text.begin() + m_SelectionStart++, clipboard[i]);
                }
            }

            m_SelectionEnd = m_SelectionStart;
            return true;

        };

        if (&current == &Internal::System->Elements[Internal::System->ActiveID]) {
            bool mutate = !m_Text.empty() && (updateSelection() || removeText());
            if (mutate || insertText()) {
                Internal::System->Time = 0.0f; // Definitely shouldn't be mutating global state here
                canMouseSelect = false;
            } else if (m_SelectionStart != m_SelectionEnd && Internal::Input->TestModKey(Internal::Keys::Control) && Internal::Input->TestInputKey(Internal::Keys::C)) {
                auto selectionBegin = m_Text.begin() + glm::min(m_SelectionStart, m_SelectionEnd);
                auto selectionEnd = m_Text.begin() + glm::max(m_SelectionStart, m_SelectionEnd);
                std::basic_string<T> selection(selectionBegin, selectionEnd);
                Clipboard::SetClipboard<T>(selection);
            }
        }

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

        if (!currentIndex || currentIndex + 2 >= UI::Internal::System->Elements.size()) {
            RC_WARN("UI element with TextInputWidget should have two descendants");
            return;
        }

        float edgeWidth = (2 * 0.075f) * glm::min(glm::abs(current.Size.x), glm::abs(current.Size.y));
        glm::vec2 innerSize = glm::vec2(1.0f) - edgeWidth / current.Size;
        
        Surface& scrollContainer = Internal::System->Elements[currentIndex + 1];
        scrollContainer.Size *= innerSize;
        auto scrollWidget = dynamic_cast<ScrollWidget*>(scrollContainer.Widget.get());
        if (scrollWidget == nullptr) {
            RC_WARN("The first child of UI element with TextInputWidget<T> should have a ScrollWidget<T> member");
            return;
        }

        Surface& textDisplay = Internal::System->Elements[currentIndex + 2];
        auto widget = dynamic_cast<TextDisplayWidget<T>*>(textDisplay.Widget.get());
        if (widget == nullptr) {
            RC_WARN("The first grandchild of UI element with TextInputWidget<T> should have a TextDisplayWidget<T> member");
            return;
        }

        widget->ColourIndex = !m_Text.empty() + (&Internal::System->Elements[Internal::System->ActiveID] == &current);
        
        textDisplay.Size *= innerSize;
            
        textDisplay.Positioning = PositioningType::Offset;
        textDisplay.Position = glm::vec2(0.0f, 0.25f);
        
        if (m_Text.empty()) {
            return;
        }

        widget->Text = std::basic_string_view<T>(m_Text.data(), m_Text.size());

        m_CaretSize = glm::vec2(Internal::Font->GetGlyphInfo(' ').Advance * 0.25f, Internal::Font->GetGlyphInfo(' ').Size.y);
        float fontSizeMultiplier = widget->TextScale * textDisplay.Size.y / m_CaretSize.y;
        float textWidth = m_CaretSize.x * 3.0f;
                
        bool swapSelection = m_SelectionEnd < m_SelectionStart;
        {
            float selectedWidth = 0.0f;
                    
            size_t selStart = (swapSelection) ? m_SelectionEnd : m_SelectionStart;
            size_t selEnd = (swapSelection) ? m_SelectionStart : m_SelectionEnd;
            for (size_t i = 0; i < selEnd; i++) {
                float charWidth = static_cast<float>(Internal::Font->GetGlyphInfo(m_Text[i]).Advance);
                    
                if (charWidth == 0) {
                    charWidth = static_cast<float>(Internal::Font->GetGlyphInfo('?').Advance);
                }
                    
                if (i < m_SelectionStart) {
                    textWidth += charWidth;
                }

                if (i >= selStart) {
                    selectedWidth += charWidth;
                }
            }

            if (selectedWidth) {
                m_CaretSize.x = selectedWidth;
            }
        }

        m_CaretSize.x *= fontSizeMultiplier;
        textWidth *= fontSizeMultiplier;
        
        float scrollOffset = (scrollWidget->m_ScrollOffset - 0.025f) * scrollContainer.Size.x + fontSizeMultiplier;
        if (textWidth - scrollOffset > scrollContainer.Size.x || textWidth - scrollOffset < m_CaretSize.x * 1.5f * (m_SelectionStart == m_SelectionEnd)) {
            scrollOffset = glm::max(0.0f, textWidth - scrollContainer.Size.x);
            scrollWidget->m_ScrollOffset = (scrollOffset - fontSizeMultiplier) / scrollContainer.Size.x + 0.025f;
        }

        m_CaretPosition = textDisplay.Size.x * -0.5f + textWidth - scrollOffset;
        m_CaretSize.y = widget->TextScale * textDisplay.Size.y * 0.75f;
        if(m_SelectionStart != m_SelectionEnd) {
            m_CaretPosition -= m_CaretSize.x * 0.5f * - (1.0f - swapSelection * 2.0f) + 0.5f * edgeWidth;

            float sign = 1.0f - 2.0f * (m_CaretPosition < 0.0f);
            float size = innerSize.x * current.Size.x * 0.5f;
            if (sign * m_CaretPosition + m_CaretSize.x * 0.5f > size) {
                m_CaretSize.x = sign * (size - sign * m_CaretPosition + m_CaretSize.x * 0.5f);
                m_CaretPosition = sign * size - m_CaretSize.x * 0.5f;
            }
        } else {
            m_CaretPosition -= m_SelectionStart == m_Text.size() ? m_CaretSize.x * 1.5f : m_CaretSize.x * 2.0f; // TODO smarter way of offsetting caret pos
        }

        if (!canMouseSelect || Internal::Input->MouseState.Left != Internal::MouseButtonState::Held) {
            return;
        }
        //Mouse selection

        glm::vec2 relativeMousePosition = (Internal::Input->MouseState.Position - current.Position);
        bool inside = glm::abs(relativeMousePosition.x) < current.Size.x * innerSize.x * 0.5f && glm::abs(relativeMousePosition.y) < current.Size.y * innerSize.y * 0.5f;
            
        if (!inside) {
            if (Internal::System->Time < 0.0f && Internal::System->Time > -0.125f) { // FIXME Definitely shouldn't be relying on unrelated global state (Internal::System::Time)
                int32_t direction = 1 - 2 * (relativeMousePosition.x < 0.0f);
                        
                if (m_SelectionStart + direction <= m_Text.size()) {
                    m_SelectionStart += direction;
                }

                Internal::System->Time = -0.2f; // Definitely shouldn't be mutating global state here
            }

            return;
        }

        float mouseOffset = (relativeMousePosition.x + textDisplay.Size.x * 0.5f + scrollOffset) / fontSizeMultiplier;
        float textOffset = 0.0f;

        size_t i = 0;
        for (; i < m_Text.size(); i++) {
            float charWidth = static_cast<float>(Internal::Font->GetGlyphInfo(m_Text[i]).Advance);
            
            if (charWidth == 0) {
                charWidth = static_cast<float>(Internal::Font->GetGlyphInfo('?').Advance);
            }

            if (textOffset + 0.5f * charWidth >= mouseOffset) {
                break;
            }

            textOffset += charWidth;
        }

        m_SelectionStart = i;

        if (Internal::System->Time >= -0.1f) {
            m_SelectionEnd = i;
        }

        Internal::System->Time = -0.2f; // Definitely shouldn't be mutating global state here
    }

    template <typename T>
    bool TextInputWidget<T>::Render(Surface& current) {
        uint32_t index = &Internal::System->Elements[Internal::System->ActiveID] == &current ? 2 : &Internal::System->Elements[Internal::System->HoverID] == &current ? 1 : 0;
        
        Renderer2D::DrawFlatRoundedQuad(current.Size * 0.99f, 0.2f, 2, { current.Position.x, current.Position.y, 0.0f }, glm::vec3(1.0f), current.Colours[index]);
        Renderer2D::DrawFlatRoundedQuadEdge(current.Size, 0.075f, 0.2f, 5, { current.Position.x, current.Position.y, 0.0f }, glm::vec3(1.0f), m_HighlightColours[index]);
        
        if (index == 2 && uint32_t(Internal::System->Time) % 2 == 0 && m_CaretSize.x * m_CaretSize.y) {
            glm::mat4 transform = glm::translate(glm::mat4(1.0f), { current.Position.x + m_CaretPosition, current.Position.y, 0.0f });
            transform = glm::scale(transform, { m_CaretSize.x, m_CaretSize.y, 0.0f });

            Renderer2D::DrawShapeQuad(0, m_HighlightColours[index], transform);
        }
        return true;
    }

    template class TextInputWidget<char>;
    template class TextInputWidget<wchar_t>;
    
    template <typename T>
    void TextureTextInputWidget<T>::Update(Surface& current) {
        m_SelectionStart = glm::min(m_SelectionStart, m_Text.size());
        m_SelectionEnd = glm::min(m_SelectionEnd, m_Text.size());
        bool canMouseSelect = true;

        auto updateSelection = [this]() -> bool {
            if (Internal::Input->TestModKey(Internal::Keys::Control) && Internal::Input->TestInputKey(Internal::Keys::A)) {
                m_SelectionStart = m_Text.size();
                m_SelectionEnd = 0;
                return true;
            }

            int64_t direction = Internal::Input->TestInputKey(Internal::Keys::Rigth) - Internal::Input->TestInputKey(Internal::Keys::Left);

            bool skipWordSearch = false;
            if (Internal::Input->TestInputKey(Internal::Keys::Home) || Internal::Input->TestInputKey(Internal::Keys::Up) || Internal::Input->TestInputKey(Internal::Keys::PageUp)) {
                direction = -1 * m_SelectionStart;
                skipWordSearch = true;
            }

            if (Internal::Input->TestInputKey(Internal::Keys::End) || Internal::Input->TestInputKey(Internal::Keys::Down) || Internal::Input->TestInputKey(Internal::Keys::PageDown)) {
                direction = m_Text.size() - m_SelectionStart;
                skipWordSearch = true;
            }

            if (!direction) {
                return false;
            }

            if (!Internal::Input->TestModKey(Internal::Keys::Shift) && m_SelectionEnd != m_SelectionStart) {
                size_t caret = direction < 0 ? glm::min(m_SelectionEnd, m_SelectionStart) : glm::max(m_SelectionEnd, m_SelectionStart);
                m_SelectionEnd = m_SelectionStart = caret;
                return true;
            }

            if (!skipWordSearch && Internal::Input->TestModKey(Internal::Keys::Control)) {
                enum class CharacterType : char8_t {
                    None = 0,
                    Space,
                    Alphanumeric,
                    Other,
                };

                int64_t offset = 0;

                size_t current = (1 + direction) / 2;
                size_t previous = 1 - current;
                std::array<CharacterType, 2> window = { CharacterType::None, CharacterType::None };
                for (size_t i = m_SelectionStart - (direction < 0); direction && i < m_Text.size(); i += direction) {
                    uint32_t c = m_Text[i];

                    window[previous] = window[current];
                    window[current] = c == ' ' ? CharacterType::Space : (c > 0xBF || (c >= 'A' && c <= 'z') || (c >= '0' && c <= '9')) ? CharacterType::Alphanumeric : CharacterType::Other;

                    if (window[0] == CharacterType::Space && window[1] > CharacterType::Space) {
                        break;
                    }

                    if ((char8_t)window[0] + (char8_t)window[1] == (char8_t)CharacterType::Alphanumeric + (char8_t)CharacterType::Other) {
                        break;
                    }

                    offset += direction;
                }

                if (offset) {
                    direction = offset;
                }
            }

            m_SelectionStart = glm::clamp<int64_t>(m_SelectionStart + direction, 0, m_Text.size());
            if (!Internal::Input->TestModKey(Internal::Keys::Shift)) {
                m_SelectionEnd = m_SelectionStart;
            }

            return true;
        };
        auto removeText = [this]() -> bool {
            size_t offset = Internal::Input->TestInputKey(Internal::Keys::Backspace) ? 1 : Internal::Input->TestInputKey(Internal::Keys::Delete) ? 0 : 2;
            bool cut = Internal::Input->TestModKey(Internal::Keys::Control) && Internal::Input->TestInputKey(Internal::Keys::X) && m_SelectionStart != m_SelectionEnd;

            if (offset == 2 && !cut) {
                return false;
            }

            if (m_SelectionStart != m_SelectionEnd) {
                size_t min = glm::min(m_SelectionStart, m_SelectionEnd);
                auto selectionEnd = m_Text.begin() + glm::max(m_SelectionStart, m_SelectionEnd);

                if (cut) {
                    std::basic_string<T> selection(m_Text.begin() + min, selectionEnd);
                    Clipboard::SetClipboard<T>(selection);
                }

                m_Text.erase(m_Text.begin() + min, selectionEnd);
                m_SelectionStart = m_SelectionEnd = min;
                return true;
            }

            size_t eraseAt = m_SelectionStart - offset;
            if (eraseAt < m_Text.size()) {
                m_Text.erase(m_Text.begin() + eraseAt);
                m_SelectionStart = m_SelectionEnd = eraseAt;
                return true;
            }

            return false;
        };
        auto insertText = [this]() -> bool {
            size_t inputSize = Internal::Input->KeyboardState.InputedText.size();
            bool paste = Internal::Input->TestModKey(Internal::Keys::Control) && Internal::Input->TestInputKey(Internal::Keys::V);

            if (!inputSize && !paste) {
                return false;
            }

            if (m_SelectionStart != m_SelectionEnd) {
                size_t min = glm::min(m_SelectionStart, m_SelectionEnd);
                m_Text.erase(m_Text.begin() + min, m_Text.begin() + glm::max(m_SelectionStart, m_SelectionEnd));
                m_SelectionStart = m_SelectionEnd = min;
            }

            if (m_Text.size() == m_Text.capacity()) {
                return false;
            }

            for (size_t i = 0; i < inputSize && m_Text.size() < m_Text.capacity(); i++) {
                m_Text.emplace(m_Text.begin() + m_SelectionStart++, Internal::Input->KeyboardState.InputedText[i]);
            }

            if (!inputSize) {
                std::basic_string<T> clipboard = Clipboard::GetClipboard<T>();

                for (size_t i = 0; i < clipboard.size() && m_Text.size() < m_Text.capacity(); i++) {
                    m_Text.emplace(m_Text.begin() + m_SelectionStart++, clipboard[i]);
                }
            }

            m_SelectionEnd = m_SelectionStart;
            return true;
        };

        if (&current == &Internal::System->Elements[Internal::System->ActiveID]) {
            bool mutate = !m_Text.empty() && (updateSelection() || removeText());
            if (mutate || insertText()) {
                Internal::System->Time = 0.0f; // Definitely shouldn't be mutating global state here
                canMouseSelect = false;
            } else if (m_SelectionStart != m_SelectionEnd && Internal::Input->TestModKey(Internal::Keys::Control) && Internal::Input->TestInputKey(Internal::Keys::C)) {
                auto selectionBegin = m_Text.begin() + glm::min(m_SelectionStart, m_SelectionEnd);
                auto selectionEnd = m_Text.begin() + glm::max(m_SelectionStart, m_SelectionEnd);
                std::basic_string<T> selection(selectionBegin, selectionEnd);
                Clipboard::SetClipboard<T>(selection);
            }
        }

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

        if (!currentIndex || currentIndex + 3 >= UI::Internal::System->Elements.size()) {
            RC_WARN("UI element with TextureTextInputWidget should have three descendants");
            return;
        }

        float edgeWidth = (2 * 0.075f) * glm::min(glm::abs(current.Size.x), glm::abs(current.Size.y));
        glm::vec2 innerSize = glm::vec2(1.0f) - edgeWidth / current.Size;

        Surface& scrollContainer = Internal::System->Elements[currentIndex + 1];
        scrollContainer.Size *= innerSize;
        auto scrollWidget = dynamic_cast<ScrollWidget*>(scrollContainer.Widget.get());
        if (scrollWidget == nullptr) {
            RC_WARN("The first child of UI element with TextureTextInputWidget<T> should have a ScrollWidget<T> member");
            return;
        }
        
        Surface& textDisplay = Internal::System->Elements[currentIndex + 2];
        auto widget = dynamic_cast<TextDisplayWidget<T>*>(textDisplay.Widget.get());
        if (widget == nullptr) {
            RC_WARN("The first grandchild of UI element with TextureTextInputWidget<T> should have a TextDisplayWidget<T> member");
            return;
        }

        widget->ColourIndex = !m_Text.empty() + (&Internal::System->Elements[Internal::System->ActiveID] == &current);

        textDisplay.Size *= innerSize;

        textDisplay.Positioning = PositioningType::Offset;
        textDisplay.Position = glm::vec2(0.0f, 0.25f);

        if (m_Text.empty()) {
            return;
        }

        widget->Text = std::basic_string_view<T>(m_Text.data(), m_Text.size());

        m_CaretSize = glm::vec2(Internal::Font->GetGlyphInfo(' ').Advance * 0.25f, widget->TextScale * textDisplay.Size.y * 0.75f);
        float fontSizeMultiplier = widget->TextScale * textDisplay.Size.y / Internal::Font->GetGlyphInfo(' ').Size.y;
        float textWidth = m_CaretSize.x * 3.0f;

        bool swapSelection = m_SelectionEnd < m_SelectionStart;
        {
            float selectedWidth = 0.0f;

            size_t selStart = (swapSelection) ? m_SelectionEnd : m_SelectionStart;
            size_t selEnd = (swapSelection) ? m_SelectionStart : m_SelectionEnd;
            for (size_t i = 0; i < selEnd; i++) {
                float charWidth = static_cast<float>(Internal::Font->GetGlyphInfo(m_Text[i]).Advance);

                if (charWidth == 0) {
                    charWidth = static_cast<float>(Internal::Font->GetGlyphInfo('?').Advance);
                }

                if (i < m_SelectionStart) {
                    textWidth += charWidth;
                }

                if (i >= selStart) {
                    selectedWidth += charWidth;
                }
            }

            if (selectedWidth) {
                m_CaretSize.x = selectedWidth;
            }
        }

        m_CaretSize.x *= fontSizeMultiplier;
        textWidth *= fontSizeMultiplier;

        float caretWidth = m_SelectionStart == m_SelectionEnd ? m_CaretAspectRatio * m_CaretSize.y : 0.0f;
        float scrollOffset = (scrollWidget->m_ScrollOffset - 0.025f) * scrollContainer.Size.x + fontSizeMultiplier;

        //Update scroll offset if caret is too far to either side
        bool right = textWidth + caretWidth - scrollOffset > scrollContainer.Size.x;
        if (right || textWidth - scrollOffset < m_CaretSize.x * 1.5f * (m_SelectionStart == m_SelectionEnd)) {
            scrollOffset = glm::max(0.0f, textWidth + right * caretWidth - scrollContainer.Size.x);
            scrollWidget->m_ScrollOffset = (scrollOffset - fontSizeMultiplier) / scrollContainer.Size.x + 0.025f;
        }
        
        m_CaretPosition = textDisplay.Size.x * -0.5f + textWidth - scrollOffset;
        if (m_SelectionStart != m_SelectionEnd) {
            m_CaretPosition -= m_CaretSize.x * 0.5f * -(1.0f - swapSelection * 2.0f) + 0.5f * edgeWidth;

            float sign = 1.0f - 2.0f * (m_CaretPosition < 0.0f);
            float size = innerSize.x * current.Size.x * 0.5f;
            if (sign * m_CaretPosition + m_CaretSize.x * 0.5f > size) {
                m_CaretSize.x = sign * (size - sign * m_CaretPosition + m_CaretSize.x * 0.5f);
                m_CaretPosition = sign * size - m_CaretSize.x * 0.5f;
            }
        } else {
            Surface& caretSurface = Internal::System->Elements[currentIndex + 3];
            if (dynamic_cast<AtlasTextureWidget*>(caretSurface.Widget.get()) == nullptr) {
                 RC_WARN("The second child of UI element with TextureTextInputWidget should have a AtlasTextureWidget member");
                return;
            }

            if (&Internal::System->Elements[Internal::System->ActiveID] == &current) {
                m_CaretPosition += caretWidth * 0.25f;

                caretSurface.Size.x = caretWidth;
                caretSurface.Size.y = m_CaretSize.y;

                caretSurface.Position.x = -m_CaretPosition / current.Size.x;
            }
        }

        if (!canMouseSelect || Internal::Input->MouseState.Left != Internal::MouseButtonState::Held) {
            return;
        }
        //Mouse selection

        glm::vec2 relativeMousePosition = (Internal::Input->MouseState.Position - current.Position);
        bool inside = glm::abs(relativeMousePosition.x) < current.Size.x * innerSize.x * 0.5f && glm::abs(relativeMousePosition.y) < current.Size.y * innerSize.y * 0.5f;

        if (!inside) {
            if (Internal::System->Time < 0.0f && Internal::System->Time > -0.125f) { // FIXME Definitely shouldn't be relying on unrelated global state (Internal::System::Time)
                int32_t direction = 1 - 2 * (relativeMousePosition.x < 0.0f);

                if (m_SelectionStart + direction <= m_Text.size()) {
                    m_SelectionStart += direction;
                }

                Internal::System->Time = -0.2f; // Definitely shouldn't be mutating global state here
            }

            return;
        }

        float mouseOffset = (relativeMousePosition.x + textDisplay.Size.x * 0.5f + scrollOffset) / fontSizeMultiplier;
        float textOffset = 0.0f;

        size_t i = 0;
        for (; i < m_Text.size(); i++) {
            float charWidth = static_cast<float>(Internal::Font->GetGlyphInfo(m_Text[i]).Advance);

            if (charWidth == 0) {
                charWidth = static_cast<float>(Internal::Font->GetGlyphInfo('?').Advance);
            }

            if (textOffset + 0.5f * charWidth >= mouseOffset) {
                break;
            }

            textOffset += charWidth;
        }

        m_SelectionStart = i;

        if (Internal::System->Time >= -0.1f) {
            m_SelectionEnd = i;
        }

        Internal::System->Time = -0.2f; // Definitely shouldn't be mutating global state here
    }


    template <typename T>
    bool TextureTextInputWidget<T>::Render(Surface& current) {
        uint32_t index = &Internal::System->Elements[Internal::System->ActiveID] == &current ? 2 : &Internal::System->Elements[Internal::System->HoverID] == &current ? 1 : 0;

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), { current.Position.x, current.Position.y, 0.0f });
        transform = glm::scale(transform, { current.Size.x, current.Size.y, 0.0f });

        glm::vec2 atlasOffset(m_BoxAtlasIndices[index] % (uint32_t)Internal::AtlasSize.x, m_BoxAtlasIndices[index] / (uint32_t)Internal::AtlasSize.x * -1.0f);

        glm::mat3 texTransform = glm::translate(glm::mat3(1.0f), atlasOffset / Internal::AtlasSize);
        texTransform = glm::scale(texTransform, glm::vec2(m_BoxScale.x, -m_BoxScale.y) / Internal::AtlasSize);

        Renderer2D::DrawShapeQuad(3, current.Colours[index], transform, texTransform);

        if (index != 2 || !m_CaretSize.y) {
            return true;
        }

        if (m_SelectionStart == m_SelectionEnd) {
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

            if (!currentIndex || currentIndex + 3 >= UI::Internal::System->Elements.size()) {
                RC_WARN("UI element with TextureTextInputWidget should have three descendants");
                return true;
            }

            Surface& caretSurface = Internal::System->Elements[currentIndex + 3];
            if (dynamic_cast<AtlasTextureWidget*>(caretSurface.Widget.get()) == nullptr) {
                RC_WARN("The second child of UI element with TextureTextInputWidget should have a AtlasTextureWidget member");
                return true;
            }

            caretSurface.Size.x *= uint32_t(Internal::System->Time) % 2 == 0;
            return true;
        } 

        float selectionEndsWidth = m_CaretSize.y * m_SelectionEndsScale.x / m_SelectionEndsScale.y;

        if (glm::abs(m_CaretSize.x) > selectionEndsWidth) {
            //Selection middle
            transform = glm::translate(glm::mat4(1.0f), { current.Position.x + m_CaretPosition, current.Position.y, 0.0f });
            transform = glm::scale(transform, { glm::abs(m_CaretSize.x) - 2.0f * selectionEndsWidth, m_CaretSize.y, 0.0f });

            atlasOffset = glm::vec2(m_SelectionAtlasIndices[1] % (uint32_t)Internal::AtlasSize.x, m_SelectionAtlasIndices[1] / (uint32_t)Internal::AtlasSize.x * -1.0f);

            texTransform = glm::translate(glm::mat3(1.0f), atlasOffset / Internal::AtlasSize);
            texTransform = glm::scale(texTransform, glm::vec2(m_SelectionMiddleScale.x, -m_SelectionMiddleScale.y) / Internal::AtlasSize);

            Renderer2D::DrawShapeQuad(3, current.Colours[index], transform, texTransform);
        
            //Selection rigth
            transform = glm::translate(glm::mat4(1.0f), { current.Position.x + m_CaretPosition + 0.5f * (glm::abs(m_CaretSize.x) - selectionEndsWidth), current.Position.y, 0.0f });
            transform = glm::scale(transform, { selectionEndsWidth, m_CaretSize.y, 0.0f });

            atlasOffset = glm::vec2(m_SelectionAtlasIndices[2] % (uint32_t)Internal::AtlasSize.x, m_SelectionAtlasIndices[2] / (uint32_t)Internal::AtlasSize.x * -1.0f);

            texTransform = glm::translate(glm::mat3(1.0f), atlasOffset / Internal::AtlasSize);
            texTransform = glm::scale(texTransform, glm::vec2(m_SelectionEndsScale.x, -m_SelectionEndsScale.y) / Internal::AtlasSize);

            Renderer2D::DrawShapeQuad(3, current.Colours[index], transform, texTransform);
        }

        //Selection left
        transform = glm::translate(glm::mat4(1.0f), { current.Position.x + m_CaretPosition - 0.5f * (glm::abs(m_CaretSize.x) - selectionEndsWidth), current.Position.y, 0.0f });
        transform = glm::scale(transform, { selectionEndsWidth, m_CaretSize.y, 0.0f });

        atlasOffset = glm::vec2(m_SelectionAtlasIndices[0] % (uint32_t)Internal::AtlasSize.x, m_SelectionAtlasIndices[0] / (uint32_t)Internal::AtlasSize.x * -1.0f);

        texTransform = glm::translate(glm::mat3(1.0f), atlasOffset / Internal::AtlasSize);
        texTransform = glm::scale(texTransform, glm::vec2(m_SelectionEndsScale.x, -m_SelectionEndsScale.y) / Internal::AtlasSize);

        Renderer2D::DrawShapeQuad(3, current.Colours[index], transform, texTransform);
        

        return true;
    }

    template class TextureTextInputWidget<char>;
    template class TextureTextInputWidget<wchar_t>;
    
    template <typename ValueType, typename CharType>
    void NumericInputWidget<ValueType, CharType>::Update(Surface& current) {
        if (Internal::System->ActiveID != m_TextInputID) {
            if (m_Text.empty()) {
                std::basic_string<CharType> valueString = m_ValueTypeToString(m_Value);

                for (size_t i = 0; i < valueString.size() && m_Text.size() < m_Text.capacity(); i++) {
                    m_Text.emplace_back(valueString[i]);
                }
            }

            return;
        }

        if (!Internal::Input->TestInputKey(Internal::Keys::Enter)) {
            return;
        }

        try {
            ValueType val = m_ValueTypeFromString({ m_Text.begin(), m_Text.end() });

            std::optional<ValueType> validated = m_ValidateValue(val);
            if (!validated) {
                throw 1;
            }
            
            m_Value = validated.value();
            Internal::System->ActiveID = 0;

        } catch (...) {}

        m_Text.clear();

        std::basic_string valueString = m_ValueTypeToString(m_Value);
        for (size_t i = 0; i < valueString.size() && m_Text.size() < m_Text.capacity(); i++) {
            m_Text.emplace_back(valueString[i]);
        }
    }

    template class NumericInputWidget<int32_t, char>;
    template class NumericInputWidget<int32_t, wchar_t>;
    template class NumericInputWidget<int64_t, char>;
    template class NumericInputWidget<int64_t, wchar_t>;
    template class NumericInputWidget<uint64_t, char>;
    template class NumericInputWidget<uint64_t, wchar_t>;
    template class NumericInputWidget<float, char>;
    template class NumericInputWidget<float, wchar_t>;
    template class NumericInputWidget<double, char>;
    template class NumericInputWidget<double, wchar_t>;

    bool ToggleWidget::Render(Surface& current) {
        constexpr glm::vec3 AxisZ(0.0f, 0.0f, 1.0f);

        uint32_t index = &Internal::System->Elements[Internal::System->ActiveID] == &current ? 2 : &Internal::System->Elements[Internal::System->HoverID] == &current ? 1 : 0;

        Renderer2D::DrawFlatRoundedQuadEdge(current.Size, 0.125f, 0.2f, 5, { current.Position.x, current.Position.y, 0.0f }, glm::vec3(1.0f), current.Colours[index]);

        if (m_Enabled) {
            const glm::vec4& colour = m_Colours[index];
            const glm::vec2 size(glm::min(current.Size.x * 0.2f, current.Size.y * 0.25f), current.Size.y * 0.8f);
            Renderer2D::DrawRotatedFlatRoundedQuad(size, 0.5f, 5, { current.Position.x, current.Position.y, 0.0f }, 45.0f, AxisZ, glm::vec3(1.0f), colour);
            Renderer2D::DrawRotatedFlatRoundedQuad(size, 0.5f, 5, { current.Position.x, current.Position.y, 0.0f }, -45.0f, AxisZ, glm::vec3(1.0f), colour);
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
        double nextValue = (Internal::Input->MouseState.Position[m_SliderDimension] - current.Position[m_SliderDimension] + current.Size[m_SliderDimension] * 0.5f * maxPosition) / (current.Size[m_SliderDimension] * maxPosition) * (m_Max - m_Min) + m_Min;
        if constexpr (std::is_integral<T>::value) {
            nextValue = glm::round(nextValue);
        }

        m_Value = glm::clamp<T>(static_cast<T>(nextValue), m_Min, m_Max);
    }

    template <typename T>
    bool SliderWidget<T>::Render(Surface& current) {
        uint32_t index = &Internal::System->Elements[Internal::System->ActiveID] == &current ? 2 : &Internal::System->Elements[Internal::System->HoverID] == &current ? 1 : 0;
        const glm::vec4& colour = current.Colours[index];

        Renderer2D::DrawFlatRoundedQuad(current.Size, 0.2f, 2, { current.Position.x, current.Position.y, 0.0f }, glm::vec3(1.0f), colour * 0.8f);

        glm::vec3 sliderPosition(0.0f);
        float maxPosition = current.Size[m_SliderDimension] * (1.0f - m_SliderSize) - 0.125f * glm::min(glm::abs(current.Size.x), glm::abs(current.Size.y));
        sliderPosition[m_SliderDimension] = current.Position[m_SliderDimension] + maxPosition * glm::clamp(static_cast<float>(m_Value - m_Min) / static_cast<float>(m_Max - m_Min) - 0.5f, -0.5f, 0.5f);
        sliderPosition[1 - m_SliderDimension] = current.Position[1 - m_SliderDimension];
        
        glm::vec3 sliderSize(0.0f);
        sliderSize[m_SliderDimension] = current.Size[m_SliderDimension] * m_SliderSize;
        sliderSize[1 - m_SliderDimension] = current.Size[1 - m_SliderDimension] * 0.85f;
        
        Renderer2D::DrawFlatShapeQuad(sliderPosition, sliderSize, m_SliderColours[index]);
        
        Renderer2D::DrawFlatRoundedQuadEdge(current.Size, 0.075f, 0.2f, 5, { current.Position.x, current.Position.y, 0.0f }, glm::vec3(1.0f), { colour.r * 1.2f, colour.g * 1.2f, colour.b * 1.2f, colour.a });

        return true;
    }

    template class SliderWidget<int8_t>;
    template class SliderWidget<uint8_t>;
    template class SliderWidget<int32_t>;
    template class SliderWidget<uint32_t>;
    template class SliderWidget<int64_t>;
    template class SliderWidget<uint64_t>;
    template class SliderWidget<float>;
    template class SliderWidget<double>;

    template <typename T>
    void AtlasTextureSliderWidget<T>::Update(Surface& current) {
        if (&Internal::System->Elements[Internal::System->ActiveID] != &current) {
            return;
        }

        double nextValue = (Internal::Input->MouseState.Position[m_SliderDimension] - current.Position[m_SliderDimension] + current.Size[m_SliderDimension] * 0.5f * (1.0f - m_SliderSize[m_SliderDimension])) / (current.Size[m_SliderDimension] * (1.0f - m_SliderSize[m_SliderDimension])) * (m_Max - m_Min) + m_Min;
        if constexpr (std::is_integral<T>::value) {
            nextValue = glm::round(nextValue);
        }

        m_Value = glm::clamp<T>(static_cast<T>(nextValue), m_Min, m_Max);
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
        sliderPosition[m_SliderDimension] = current.Position[m_SliderDimension] + (current.Size[m_SliderDimension] * (1.0f - m_SliderSize[m_SliderDimension])) * glm::clamp(static_cast<float>(m_Value + m_Min) / static_cast<float>(m_Max - m_Min) - 0.5f, -0.5f, 0.5f);
        sliderPosition[1 - m_SliderDimension] = current.Position[1 - m_SliderDimension];

        transform = glm::translate(glm::mat4(1.0f), sliderPosition);
        transform = glm::scale(transform, { m_SliderSize.x * current.Size.x, m_SliderSize.y * current.Size.y, 0.0f });
    
        atlasOffset = glm::vec2(m_SliderAtlasIndices[index] % (uint32_t)Internal::AtlasSize.x, m_SliderAtlasIndices[index] / (uint32_t)Internal::AtlasSize.x * -1.0f);
        texTransform = glm::translate(glm::mat3(1.0f), atlasOffset / Internal::AtlasSize);
        texTransform = glm::scale(texTransform, glm::vec2(m_SliderScale.x, -m_SliderScale.y) / Internal::AtlasSize);

        Renderer2D::DrawShapeQuad(3, current.Colours[index], transform, texTransform);
        
        return true;
    }

    template class AtlasTextureSliderWidget<int8_t>;
    template class AtlasTextureSliderWidget<uint8_t>;
    template class AtlasTextureSliderWidget<int32_t>;
    template class AtlasTextureSliderWidget<uint32_t>;
    template class AtlasTextureSliderWidget<int64_t>;
    template class AtlasTextureSliderWidget<uint64_t>;
    template class AtlasTextureSliderWidget<float>;
    template class AtlasTextureSliderWidget<double>;
    
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
        auto scrollWidget = dynamic_cast<ScrollWidget*>(parent.Widget.get());
        if (scrollWidget) {
            float scrollSize = scrollWidget->m_ScrollSize;
            
            if (moveDirection != 0) {
                m_ScrollOffset += moveDirection * SCROLLSTEP;
                m_ScrollOffset = glm::clamp(m_ScrollOffset, 0.0f, scrollSize);
            }

            Surface& child = Internal::System->Elements[currentIndex + 1];
            auto sliderWidget = dynamic_cast<SliderWidget<float>*>(child.Widget.get());
            if(sliderWidget) {
                sliderWidget->m_Max = scrollSize;

                uint32_t scrollDimension = scrollWidget->m_ScrollDimension;
                sliderWidget->m_SliderSize = glm::clamp(glm::abs(parent.Size[scrollDimension] / scrollSize), 0.05f, 0.9f);
            } else {
                RC_WARN("The first child of UI element with ScrollBarWidget should have a SliderWidget member");
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
        auto scrollWidget = dynamic_cast<ScrollWidget*>(parent.Widget.get());
        if (scrollWidget) {
            float scrollSize = scrollWidget->m_ScrollSize;
            
            if (moveDirection != 0) {
                m_ScrollOffset += moveDirection * SCROLLSTEP;
                m_ScrollOffset = glm::clamp(m_ScrollOffset, 0.0f, scrollSize);
            }

            Surface& child = Internal::System->Elements[currentIndex + 1];
            auto sliderWidget = dynamic_cast<AtlasTextureSliderWidget<float>*>(child.Widget.get());
            if(sliderWidget) {
                sliderWidget->m_Max = scrollSize;
            } else {
                RC_WARN("The first child of UI element with AtlasTextureScrollBarWidget should have a AtlasTextureSliderWidget member");
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

    void HoverWidget::Update(Surface& current) {
        size_t parentIndex = current.ParentID;
        size_t currentIndex = parentIndex + 1;
        
        //Get the index of the current element
        for (; currentIndex && currentIndex < UI::Internal::System->Elements.size(); currentIndex = UI::Internal::System->Elements[currentIndex].SiblingID) {
            if (&UI::Internal::System->Elements[currentIndex] == &current) {
                break;
            }
        }

        if (Internal::System->HoverID > parentIndex) {
            if (Internal::System->HoverID != currentIndex || (m_PreviousHoverID >= parentIndex && m_PreviousHoverID <= currentIndex)) {
                return;
            }

            Internal::System->HoverID = 0;
        }

        //Hide the the children of this element
        for (size_t i = currentIndex + 1; i < UI::Internal::System->Elements.size() && UI::Internal::System->Elements[i].ParentID >= currentIndex; i++) {
            UI::Internal::System->Elements[i].Size = glm::vec2(0.0f);
        }

        //Hide this element
        current.Size = glm::vec2(0.0f);
    }
}
