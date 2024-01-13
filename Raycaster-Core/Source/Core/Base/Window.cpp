#include "Window.h"

#include <iostream>

namespace Core {
	static bool s_GLFWInitialized = false;

	Window::Window(const WindowProperties& properties) {
		m_Data.Tittle = properties.Tittle;
		m_Data.Width = properties.Width;
		m_Data.Height = properties.Height;

		if (!s_GLFWInitialized) {
			glfwInit();
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

			s_GLFWInitialized = true;
		}

		m_Window = glfwCreateWindow(properties.Width, properties.Height, properties.Tittle.c_str(), nullptr, nullptr);
		glfwMakeContextCurrent(m_Window);
		
		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);
	}

	Window::~Window() {
		glfwDestroyWindow(m_Window);
	}

	Window* Window::Create(const WindowProperties& properties) {
		return new Window(properties);
	}

	void Window::OnUpdate() {
		glfwSwapBuffers(m_Window);
		glfwPollEvents();
	}

	void Window::SetVSync(bool enabled) {
		if (enabled) {
			glfwSwapInterval(1);
		} else {
			glfwSwapInterval(0);
		}

		m_Data.VSyncEnabled = enabled;
	}
}