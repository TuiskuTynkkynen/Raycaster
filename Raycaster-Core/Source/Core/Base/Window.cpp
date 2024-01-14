#include "Window.h"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
#include "Core/Events/WindowEvent.h"

#include <glad/glad.h>

#include <iostream>

namespace Core {
	static bool s_GLFWInitialized = false;

	static void GLFWErrorCallback(int error, const char* desciption) {
		std::cout << "GLFW ERROR (" << error << ") " << desciption << std::endl;
	}

	Window::Window(const WindowProperties& properties) {
		m_Data.Tittle = properties.Tittle;
		m_Data.Width = properties.Width;
		m_Data.Height = properties.Height;

		if (!s_GLFWInitialized) {
			glfwInit();
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

			glfwSetErrorCallback(GLFWErrorCallback);

			s_GLFWInitialized = true;
		}

		m_Window = glfwCreateWindow(properties.Width, properties.Height, properties.Tittle.c_str(), nullptr, nullptr);
		glfwMakeContextCurrent(m_Window);
		
		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		
		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowResize event(width, height);
			
			data.Width = width;
			data.Height = height;

			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			WindowClose event;
			data.EventCallback(event);
		});

		glfwSetWindowFocusCallback(m_Window, [](GLFWwindow* window, int32_t focused) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			if (focused == GLFW_TRUE) {
				WindowFocus event;
				data.EventCallback(event);
			} else {
				WindowLostFocus event;
				data.EventCallback(event);
			}
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int modifiers){
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS: {
				KeyPressed event(key, false);
				data.EventCallback(event);
				}
				break;
			case GLFW_REPEAT: {
				KeyPressed event(key, true);
				data.EventCallback(event);
				}
				break;
			}
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			if (action == GLFW_PRESS) {
				MouseButtonPressed event (button);
				data.EventCallback(event);
			}
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double posX, double posY) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMoved event(posX, posY);
			data.EventCallback(event);
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double offsetX, double offsetY) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolled event(offsetX, offsetY);
			data.EventCallback(event);
		});
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