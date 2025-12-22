#pragma once

#include "InputCodes.h"

#include <utility>
#include <cstdint>

namespace Core {
	class Input
	{
	public:
		static bool IsKeyPressed(uint32_t key);
		static bool IsButtonPressed(uint32_t button);
		static bool IsButtonReleased(uint32_t button);

		static float GetMouseX();
		static float GetMouseY();
		static std::pair<float, float> GetMousePos();
	};
}

