#include "Layout.h"

#include "Internal.h"

#include "Core/Debug/Debug.h"


namespace Core::UI {
    void NoLayout::Next(Surface& s) {
        glm::vec2& position = s.Position;

        switch (s.Positioning){
        case Core::UI::PositioningType::Auto:
            position = m_Position;
        break;
        case Core::UI::PositioningType::Offset:
            position = m_Position - s.Position * Internal::System->Elements[s.ParentID].Size;
            break;
        case Core::UI::PositioningType::Relative:
            s.Position += Internal::System->Elements[s.ParentID].Position;
            return;
        case Core::UI::PositioningType::Absolute:
            return;
        default:
            RC_WARN("Invalid positioning type");
            break;
        }
    }

    LinearLayout::LinearLayout(size_t currentIndex) {
        RC_ASSERT(UI::Internal::System, "Tried to create a UI layout before initializing UI system");
        RC_ASSERT(currentIndex < Internal::System->Elements.size(), "currentIndex supplied to LinearLayout constructor must be smaller than UI element count");
        
        const Surface& parent = Internal::System->Elements[Internal::System->Elements[currentIndex].ParentID];
        RC_ASSERT(parent.Layout == LayoutType::Vertical || parent.Layout == LayoutType::Horizontal || parent.Layout == LayoutType::CropVertical || parent.Layout == LayoutType::CropHorizontal, "Parent of current element supplied to LinearLayout constructor must have a Vertical or Horizontal layout");

        m_PaddingDimension = parent.Layout == LayoutType::Vertical || parent.Layout == LayoutType::CropVertical;
        m_Padding = parent.Size[m_PaddingDimension];
        
        uint32_t paddingCount = 1;
        uint32_t paddedChildCount = 1;
        for (size_t i = Internal::System->Elements[currentIndex].ParentID + 1; i && i < Internal::System->Elements.size(); i = Internal::System->Elements[i].SiblingID) {
            const Surface& current = Internal::System->Elements[i];
            
            if (current.Positioning >= PositioningType::Relative) {
                continue;
            }
            
            if (i < currentIndex) {
                m_RelativePosition += current.Size[m_PaddingDimension];
                paddingCount++;
            }
            m_Padding -= current.Size[m_PaddingDimension];
            paddedChildCount++;
        }

        m_Padding = std::max(0.025f * parent.Size[m_PaddingDimension], m_Padding / paddedChildCount);
        m_RelativePosition += paddingCount * m_Padding;
    }

    void LinearLayout::Next(Surface& s) {
        const Surface& parent = Internal::System->Elements[s.ParentID];

        glm::vec2 positioning(0.0f);
        switch (s.Positioning)
        {
        case Core::UI::PositioningType::Auto:
            break;
        case Core::UI::PositioningType::Offset:
            positioning = s.Position * parent.Size;
            break;
        case Core::UI::PositioningType::Relative:
            s.Position += parent.Position;
            return;
        case Core::UI::PositioningType::Absolute:
            return;
        default:
            RC_WARN("Invalid positioning type");
            break;
        }

        s.Position[1 - m_PaddingDimension] = parent.Position[1 - m_PaddingDimension] + positioning[1 - m_PaddingDimension];
        s.Position[m_PaddingDimension] = parent.Position[m_PaddingDimension] - parent.Size[m_PaddingDimension] * 0.5f
            + m_RelativePosition + s.Size[m_PaddingDimension] * 0.5f + positioning[m_PaddingDimension];

        m_RelativePosition += s.Size[m_PaddingDimension] + m_Padding;
    }
}