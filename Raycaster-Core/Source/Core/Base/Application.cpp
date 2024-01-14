#include "Application.h"
#include "Core/Renderer/VertexArray.h"
#include "Core/Renderer/VertexBuffer.h"
#include "Core/Renderer/ElementBuffer.h"
#include "Core/Renderer/RenderAPI.h"

namespace Core {
	Application::Application() {
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));
	}

	Application::~Application() {
		glfwTerminate();
	}

	void Application::Run() {
		const float quadVertices[]{
		 0.5f,  0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		};

		const uint32_t quadIndices[]{
			0, 1, 2,
			1, 2, 3,
		};

		Core::VertexArray quadVAO;
		Core::VertexBuffer quadVBO(quadVertices, sizeof(quadVertices));
		Core::VertexBufferLayout quadLayout;

		quadLayout.Push<float>(3);
		quadVAO.AddBuffer(quadVBO, quadLayout);

		Core::ElementBuffer quadEBO(quadIndices, 6);

		RenderAPI api;
		api.SetClearColour(0.05f, 0.075f, 0.1f);

		while (m_Running)
		{
			api.Clear();

			api.DrawIndexed(quadVAO, 6);

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