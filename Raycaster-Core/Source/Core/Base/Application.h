#pragma once

#include "Window.h"
#include "Core/Events/WindowEvent.h"

#include <memory>

namespace Core {
	class Application
	{
	protected:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;

	public:
		Application();
		~Application();

		virtual void Run() = 0;
		void OnEvent(Event& event);
		bool OnWindowCloseEvent(WindowClose& event);
	};

	Application* CreateApplication();
}

