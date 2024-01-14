#include "GraphicsContext.h"

#include <glad/glad.h>

namespace Core {
	GraphicsContext::GraphicsContext(GLFWwindow* windowHandle) 
		: m_WindowHandle(windowHandle)
	{
		glfwMakeContextCurrent(windowHandle);

		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	}

	void GraphicsContext::SwapBuffers() {
		glfwSwapBuffers(m_WindowHandle);
	}
}