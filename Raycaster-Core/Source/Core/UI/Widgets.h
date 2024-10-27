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
}
