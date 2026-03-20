#pragma once

#include "Types.h"
#include "Core/Base/Timestep.h"
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

		Timestep Time{};
		float AspectRatio = 0.0f;
	};

	enum class MouseButtonState : uint8_t {
		None = 0, 
		Held,
		Released,
	};

	struct UIMouseState {
		glm::vec2 Position{};
		bool Moved = false;

		MouseButtonState Left = MouseButtonState::None;
		MouseButtonState Right = MouseButtonState::None;

		float ScrollOffset = 0.0f;
	};

	namespace Keys {
		enum InputKeys : char8_t {
			A = 0,
			C,
			V,
			X,
			Escape,
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
			KeyCount
		};

		enum ModKeys : char8_t {
			Shift = 0,
			Control,
			ModCount
		};
	}
	
	struct UIKeyboardState {
		std::bitset<Keys::KeyCount> InputKeys;
		std::bitset<Keys::ModCount> ModKeys;
		std::vector<uint32_t> InputedText;
	};
	
	enum class KeyboardInteraction : uint8_t {
		Inactive = 0,
		Hovered,
		Active,
		Released,
	};

	struct UIInputState {
		UIMouseState MouseState;
		UIKeyboardState KeyboardState;

		KeyboardInteraction InteractionState;
		size_t InteractionID = 0;

		inline bool TestInputKey(Keys::InputKeys key) const { return KeyboardState.InputKeys.test(key); }
		inline bool TestModKey(Keys::ModKeys mod) const { return KeyboardState.ModKeys.test(mod); }
	};

	inline std::unique_ptr<UISystem> System;
	inline std::shared_ptr<Core::Font> Font;
	inline std::shared_ptr<Core::Texture2D> TextureAtlas;
	inline glm::vec2 AtlasSize;
	inline std::unique_ptr<UIInputState> Input;
}