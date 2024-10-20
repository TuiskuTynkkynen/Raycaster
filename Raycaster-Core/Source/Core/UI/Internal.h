#pragma once

#include "Types.h"
#include "Core/Font/Font.h"

#include <vector>
#include <memory>

namespace Core::UI::Internal {
	struct UISystem {
		glm::vec2 Position;
		glm::vec2 Size;

		size_t HoverID = 0;
		size_t ActiveID = 0;

		size_t OpenElement = 0;

		std::vector<Surface> Elements;
	};

	inline std::unique_ptr<UISystem> System;
	inline std::shared_ptr<Core::Font> Font;
}