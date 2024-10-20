#pragma once

#include "Types.h"
#include "Core/Font/Font.h"

#include <vector>
#include <memory>
#include <string>
#include <codecvt>

namespace Core::UI::Internal {
	struct TextData {
		std::wstring Text;
		glm::vec2 Offset = glm::vec2(0.0f);
		float Scale = 0.0f;
		
		TextData(const std::string& text) : Text(std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(text)) {}
		TextData(const std::wstring& text) : Text(text) {}
	};

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