#pragma once

#include "Core/Events/Event.h"

#include <GLFW/glfw3.h>

#include <string>
#include <functional>

namespace Core {
	struct WindowProperties {
		std::string Tittle;
		uint32_t Width;
		uint32_t Height;

		WindowProperties(const std::string& tittle = "core", uint32_t width = 1280, uint32_t height = 720)
			: Tittle(tittle), Width(width), Height(height) {}
	};

	class Window {
		using EventCallbackFunction = std::function<void(Event&)>;

	private:
		GLFWwindow* m_Window;

		struct WindowData {
			std::string Tittle;
			uint32_t Width;
			uint32_t Height;

			bool VSyncEnabled;
			EventCallbackFunction EventCallback;
		};

		WindowData m_Data;
	public:
		Window(const WindowProperties& properties);
		~Window();

		void OnUpdate();

		int GetHeight() { return m_Data.Height;  }
		int GetWidth() { return m_Data.Width; }
		
		inline void SetEventCallback(const EventCallbackFunction& callback) { m_Data.EventCallback = callback; }
		void SetVSync(bool enbled);

		static Window* Create(const WindowProperties& properties = WindowProperties());
	};
}