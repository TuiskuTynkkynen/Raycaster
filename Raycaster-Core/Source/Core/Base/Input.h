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

	std::wstring_view KeyCodeToString(uint32_t keyCode);
}

