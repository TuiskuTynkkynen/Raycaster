#pragma once
#include "Types.h"

namespace Core::UI {
	class NoLayout {
	public:
		NoLayout(const Surface& parent) {
			m_Position = parent.Position;
		}

		void Next(Surface& s) const;
	private:
		glm::vec3 m_Position;
	};

	class LinearLayout {
	public:
		LinearLayout(size_t currentIndex);
	
		void Next(Surface& s);
	private:
		uint32_t m_PaddingDimension = 0;
		float m_Padding = 0.0f;
		float m_RelativePosition = 0.0f;
	};
}

