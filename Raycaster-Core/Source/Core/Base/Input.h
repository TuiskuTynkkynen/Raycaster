#pragma once

#include "InputCodes.h"

#include <utility>
#include <cstdint>
#include <string_view>

namespace Core::Input {
	bool IsKeyPressed(uint32_t key);
	bool IsButtonPressed(uint32_t button);
	bool IsButtonReleased(uint32_t button);

	float GetMouseX();
	float GetMouseY();
	std::pair<float, float> GetMousePos();

	enum class CursorMode {
		Normal = 0, // Show cursor
		Hidden,     // Hide cursor
		Captured,   // Capture cursor
		Disabled,   // Hide and capture cursor
	};
	void SetCursorMode(CursorMode mode);

	constexpr bool IsValidKeyCode(uint32_t keyCode);
	constexpr bool IsValidButtonCode(uint32_t buttonCode);

	std::wstring_view KeyCodeToString(uint32_t keyCode);
	std::wstring_view ButtonCodeToString(uint32_t mouseButtonCode);
}

constexpr bool Core::Input::IsValidKeyCode(uint32_t keyCode) {
    switch (keyCode) {
    case  32: return true;
    case  39: return true;
    case  59: return true;
    case  61: return true;
    case  96: return true;
    case 161: return true;
    case 162: return true;
    }

    if (keyCode >= 44 && keyCode <= 57) return true;
    if (keyCode >= 65 && keyCode <= 93) return true;
    if (keyCode >= 256 && keyCode <= 269) return true;
    if (keyCode >= 280 && keyCode <= 284) return true;
    if (keyCode >= 290 && keyCode <= 314) return true;
    if (keyCode >= 320 && keyCode <= 336) return true;
    if (keyCode >= 340 && keyCode <= 348) return true;

    return false;
}

constexpr bool Core::Input::IsValidButtonCode(uint32_t buttonCode) {
    return buttonCode >= RC_MOUSE_BUTTON_1 && buttonCode <= RC_MOUSE_BUTTON_8;
}