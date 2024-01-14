#pragma once

#include <GLFW/glfw3.h>

namespace Core {
	class GraphicsContext
	{
	private: 
		GLFWwindow* m_WindowHandle;
	public:
		GraphicsContext(GLFWwindow* windowHandle);

		void SwapBuffers();
	};
}

