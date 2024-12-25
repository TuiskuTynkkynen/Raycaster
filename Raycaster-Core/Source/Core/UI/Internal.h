#pragma once

#include "Types.h"
#include "Core/Font/Font.h"
#include "Core/Renderer/Texture.h"

#include <vector>
#include <memory>
#include <bitset>

namespace Core::UI::Internal {
	struct UISystem {
		glm::vec2 Position{};
		glm::vec2 Size{};

		size_t HoverID = 0;
		size_t ActiveID = 0;

		size_t OpenElement = 0;

		std::vector<Surface> Elements;
	};

	enum class MouseButtonState {
		None = 0, 
		Held,
		Released,
	};

	struct UIMouseState {
		glm::vec2 Position{};
		MouseButtonState Left = MouseButtonState::None;
		MouseButtonState Right = MouseButtonState::None;

		float ScrollOffset = 0.0f;
	};

	namespace InputKeys {
		enum UIKeys {
		Escape = 0,
		Enter,
		Tab,
		Backspace,
		Delete,
		Rigth,
		Left,
		Down,
		Up,
		PageUp,
		PageDown,
		Home,
		End,
		Shift,
		Control,
		KeyCount
	};
	}
	
	struct UIKeyboardState {
		std::bitset<InputKeys::KeyCount> SpecialKeys;
		std::vector<uint32_t> InputedText;
	};

	struct UIInputState {
		UIMouseState MouseState;
		UIKeyboardState KeyboardState;
	};

	inline std::unique_ptr<UISystem> System;
	inline std::shared_ptr<Core::Font> Font;
	inline std::shared_ptr<Core::Texture2D> TextureAtlas;
	inline glm::vec2 AtlasSize;
	inline std::unique_ptr<UIInputState> Input;
}