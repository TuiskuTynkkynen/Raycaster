#include "Window.h"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
#include "Core/Events/TextEvent.h"
#include "Core/Events/WindowEvent.h"
#include "Core/Debug/Debug.h"


namespace Core {
	static bool s_GLFWInitialized = false;

	static void GLFWErrorCallback(int error, const char* desciption) {
		RC_ERROR("GLFW ERROR ({}) {}", error, desciption);
	}

	Window::Window(const WindowProperties& properties) {
		m_Data.Tittle = properties.Tittle;
		m_Data.Width = properties.Width;
		m_Data.Height = properties.Height;

		if (!s_GLFWInitialized) {
			int32_t success = glfwInit();
			RC_ASSERT(success, "Could not initialize GLFW");
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			
			#ifdef RC_DEBUG_MODE
				glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
			#endif
			
			glfwSetErrorCallback(GLFWErrorCallback);

			s_GLFWInitialized = true;
		}

		m_Window = glfwCreateWindow(properties.Width, properties.Height, properties.Tittle.c_str(), nullptr, nullptr);
		m_context = new GraphicsContext(m_Window);

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

			switch (action) {
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
			case GLFW_RELEASE: {
				KeyReleased event(key);
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
			} else {
				MouseButtonReleased event (button);
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

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, uint32_t codePoint) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			TextInput event(codePoint);
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
		m_context->SwapBuffers();

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