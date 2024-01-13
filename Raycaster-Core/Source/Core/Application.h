#pragma once

namespace Core {
	class Application
	{
	public:
		Application();
		virtual ~Application();

		virtual void Run() = 0;
	};

	Application* CreateApplication();
}

