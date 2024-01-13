#include "Application.h"

namespace Core {
	Application::Application() {
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));
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

	void Application::OnEvent(Event& event) {
		EventDispatcher dispatcer(event);
		dispatcer.Dispatch<WindowClose>(std::bind(&Application::OnWindowCloseEvent, this, std::placeholders::_1));
	}

	bool Application::OnWindowCloseEvent(WindowClose& event) {
		m_Running = false;
		return true;
	}
}