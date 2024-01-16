#include "Input.h"

#include "Application.h"

namespace Core {
	bool Input::IsKeyPressed(uint32_t key) {
		auto state = glfwGetKey(Application::GetWindow().GetWindowPointer(), key);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}
	
	bool Input::IsButtonPressed(uint32_t button) {
		auto state = glfwGetMouseButton(Application::GetWindow().GetWindowPointer(), button);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	float Input::GetMouseX() {
		double x, y;
		glfwGetCursorPos(Application::GetWindow().GetWindowPointer(), &x, &y);
		return (float)x;
	}

	float Input::GetMouseY() {
		double x, y;
		glfwGetCursorPos(Application::GetWindow().GetWindowPointer(), &x, &y);
		return (float)y;
	}

	std::pair<float, float> Input::GetMousePos() {
		double x, y;
		glfwGetCursorPos(Application::GetWindow().GetWindowPointer(), &x, &y);
		return { (float)x, (float)y };
	}
}