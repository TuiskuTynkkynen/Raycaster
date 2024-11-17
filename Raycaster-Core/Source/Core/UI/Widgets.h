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

	class ToggleWidget : public Widget {
	public:
		ToggleWidget(bool enabled) : m_Enabled(enabled) {}

		void Update(Surface& current) override {}
		bool Render(Surface& current) override;

	private:
		bool m_Enabled;
	};

	class AtlasTextureToggleWidget : public Widget {
	public:
		AtlasTextureToggleWidget(bool enabled, const glm::uvec3& boxAtlasIndices, const glm::uvec3& checkAtlasIndices, const glm::vec2& atlasScale)
			: m_Enabled(enabled), m_BoxAtlasIndices(boxAtlasIndices), m_CheckAtlasIndices(checkAtlasIndices), m_Scale(atlasScale) {}

		void Update(Surface& current) override {}
		bool Render(Surface& current) override;

	private:
		bool m_Enabled;

		glm::uvec3 m_BoxAtlasIndices;
		glm::uvec3 m_CheckAtlasIndices;
		glm::vec2 m_Scale;
	};

}
