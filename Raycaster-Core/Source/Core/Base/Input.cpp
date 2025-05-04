#include "Input.h"

#include "Application.h"

namespace Core {
	bool Input::IsKeyPressed(uint32_t key) {
		auto state = glfwGetKey(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), key);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}
	
	bool Input::IsButtonPressed(uint32_t button) {
		auto state = glfwGetMouseButton(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), button);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}
	
	bool Input::IsButtonReleased(uint32_t button) {
		auto state = glfwGetMouseButton(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), button);
		return state == GLFW_RELEASE;
	}

	float Input::GetMouseX() {
		double x, y;
		glfwGetCursorPos(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), &x, &y);
		return (float)x;
	}

	float Input::GetMouseY() {
		double x, y;
		glfwGetCursorPos(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), &x, &y);
		return (float)y;
	}

	std::pair<float, float> Input::GetMousePos() {
		double x, y;
		glfwGetCursorPos(static_cast<GLFWwindow*>(Application::GetWindow().GetWindowPointer()), &x, &y);
		return { (float)x, (float)y };
	}
}