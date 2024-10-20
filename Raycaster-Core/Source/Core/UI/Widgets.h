#pragma once

#include "Types.h"
#include "Core/Renderer/Texture.h"

#include <string>

namespace Core::UI::Widgets {
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

}
