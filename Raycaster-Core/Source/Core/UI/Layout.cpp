#include "Layout.h"

#include "Internal.h"

#include "Core/Debug/Debug.h"

namespace Core::UI {
	LinearLayout::LinearLayout(size_t currentIndex) {
		RC_ASSERT(UI::Internal::System, "Tried to create a UI layout before initializing UI system");
		RC_ASSERT(currentIndex < Internal::System->Elements.size(), "currentIndex supplied to LinearLayout constructor must be smaller than UI element count");
		
		const Surface& parent = Internal::System->Elements[Internal::System->Elements[currentIndex].ParentID];
		RC_ASSERT(parent.Layout == LayoutType::Vertical || parent.Layout == LayoutType::Horizontal, "Parent of current element supplied to LinearLayout constructor must have a Vertical or Horizontal layout");

		m_PaddingDimension = parent.Layout == LayoutType::Vertical;
		m_Padding = parent.Size[m_PaddingDimension];
		
		uint32_t paddingCount = 1;
		for (size_t i = Internal::System->Elements[currentIndex].ParentID + 1; i && i < Internal::System->Elements.size(); i = Internal::System->Elements[i].SiblingID) {
			if (i < currentIndex) {
				m_RelativePosition += Internal::System->Elements[i].Size[m_PaddingDimension];
				paddingCount++;
			}
			m_Padding -= Internal::System->Elements[i].Size[m_PaddingDimension];
		}

		m_Padding = std::max(0.025f * parent.Size[m_PaddingDimension], m_Padding / (parent.ChildCount + 1));
		m_RelativePosition += paddingCount * m_Padding;
	}

	void LinearLayout::Next(Surface& s) {
		const Surface& parent = Internal::System->Elements[s.ParentID];

		s.Position[1 - m_PaddingDimension] = parent.Position[1 - m_PaddingDimension];
		s.Position[m_PaddingDimension] = parent.Position[m_PaddingDimension] - parent.Size[m_PaddingDimension] * 0.5f
			+ m_RelativePosition + s.Size[m_PaddingDimension] * 0.5f;

		m_RelativePosition += s.Size[m_PaddingDimension] + m_Padding;
	}
}