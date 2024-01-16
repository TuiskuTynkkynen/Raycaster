#pragma once

#include "Window.h"
#include "Core/Events/WindowEvent.h"
#include "LayerStack.h"

#include <memory>

int main();

namespace Core {
	class Application
	{
	private:
		static Application* s_Instance;

		std::unique_ptr<Window> m_Window;
		LayerStack m_LayerStack;
		bool m_Running = true;
		
		bool OnWindowCloseEvent(WindowClose& event);
		bool OnWindowResizeEvent(WindowResize& event);

		void Run();
	public:
		Application();
		virtual ~Application();
		
		void OnEvent(Event& event);
		void Close();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		Window& GetWindow() { return *m_Window; }

		friend int ::main();
	};

	Application* CreateApplication();
}

