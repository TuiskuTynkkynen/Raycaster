#pragma once

#include <glm/glm.hpp>

#include <array>

namespace Core::UI {
	enum class LayoutType {
		None = 0,
		Vertical,
		Horizontal,
	};
	
	enum class SurfaceType {
		None = 0,
		Button,
	};

	struct Surface {
		SurfaceType Type = SurfaceType::None;
		LayoutType Layout = LayoutType::None;

		glm::vec2 Position;
		glm::vec2 Size;

		std::array<glm::vec4, 3> Colours; // [0] = primary, [1] = hover, [2] = clicked

		size_t ParentID = 0;
		size_t SiblingID = 0;
		uint32_t ChildCount = 0;
	};

	namespace {
		std::array<glm::vec4, 3> DefaultColours = {
			glm::vec4(0.125f, 0.125f, 0.5f, 1.0f),
			glm::vec4(0.325f, 0.325f, 0.7f, 1.0f),
			glm::vec4(0.7f, 0.7f, 0.325f, 1.0f)
		};
	}
}