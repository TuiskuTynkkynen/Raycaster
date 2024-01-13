#include "Application.h"

namespace Core {
	Application::Application() {
		m_Window = std::unique_ptr<Window>(Window::Create());
	}

	Application::~Application() {
		glfwTerminate();
	}

	void Application::Run() {
		while (m_Running)
		{
			glClearColor(0.05f, 0.075f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			m_Window->OnUpdate();
		}
	}
}