#include "Application.h"

#include "Core/Renderer/RenderAPI.h"
#include "Core/Renderer/Renderer2D.h"

namespace Core {
	Application* Application::s_Instance = nullptr;

	Application::Application() {
		if (s_Instance) {
			static_assert("ERROR: APPLICATION ALREADY CREATED");
		}
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

		RenderAPI::Init();
		Renderer2D::Init();
	}
	
	Application::~Application() {
		glfwTerminate();
	}

	void Application::Run() {
		while (m_Running)
		{
			for (auto iterator = m_LayerStack.begin(); iterator != m_LayerStack.end(); iterator++) {
				(*iterator)->OnUpdate();
			}
			m_Window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& event) {
		EventDispatcher dispatcer(event);
		dispatcer.Dispatch<WindowClose>(std::bind(&Application::OnWindowCloseEvent, this, std::placeholders::_1));
		dispatcer.Dispatch<WindowResize>(std::bind(&Application::OnWindowResizeEvent, this, std::placeholders::_1));

		for (auto iterator = m_LayerStack.end(); iterator != m_LayerStack.begin();) {
			if (event.handled) {
				break;
			}

			(*--iterator)->OnEvent(event);
		}
	}

	void Application::Close() {
		m_Running = false;
	}

	void Application::PushLayer(Layer* layer) {
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay) {
		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	bool Application::OnWindowCloseEvent(WindowClose& event) {
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResizeEvent(WindowResize& event) {
		RenderAPI::SetViewPort(0, 0, event.GetWidth(), event.GetHeight());
		return false;
	}
}