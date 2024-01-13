#pragma once

#include "Window.h"
#include "Core/Events/WindowEvent.h"

#include <memory>
#include <iostream>

namespace Core {
	class Application
	{
	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;

	public:
		Application();
		~Application();

		virtual void Run();
		void OnEvent(Event& event);
		bool OnWindowCloseEvent(WindowClose& event);
	};

	Application* CreateApplication();
}

