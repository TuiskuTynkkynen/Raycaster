#include "Application.h"

namespace Core {
	Application::Application() {
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));
	}

	Application::~Application() {
		glfwTerminate();
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