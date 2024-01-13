#pragma once

#include <GLFW/glfw3.h>

#include <string>

namespace Core {
	struct WindowProperties {
		std::string Tittle;
		uint32_t Width;
		uint32_t Height;

		WindowProperties(const std::string& tittle = "core", uint32_t width = 1280, uint32_t height = 720)
			: Tittle(tittle), Width(width), Height(height) {}
	};

	class Window {
	private:
		GLFWwindow* m_Window;

		struct WindowData {
			std::string Tittle;
			uint32_t Width;
			uint32_t Height;

			bool VSyncEnabled;
		};

		WindowData m_Data;
	public:
		Window(const WindowProperties& properties);
		~Window();

		int GetHeight() { return m_Data.Height;  }
		int GetWidth() { return m_Data.Width; }

		void OnUpdate();
		void SetVSync(bool enbled);

		static Window* Create(const WindowProperties& properties = WindowProperties());
	};
}