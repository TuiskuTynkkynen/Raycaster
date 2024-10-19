#pragma once

#include "Types.h"

#include <vector>

namespace Core::UI {
	class Layout {
	public:
		virtual ~Layout(){}

		virtual void Next(Surface& s) = 0;
	};

	class NoLayout : public Layout {
	public:
		NoLayout(const Surface& parent) {
			m_Position = parent.Position;
		}

		void Next(Surface& s) override;
	private:
		glm::vec2 m_Position;
	};

	class LinearLayout : public Layout {
	public:
		LinearLayout(size_t currentIndex);
	
		void Next(Surface& s) override;
	private:
		size_t m_PaddingDimension = 0;
		float m_Padding = 0.0f;
		float m_RelativePosition = 0.0f;
	};
}

