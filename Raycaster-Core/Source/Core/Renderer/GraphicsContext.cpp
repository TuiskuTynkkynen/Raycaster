#include "GraphicsContext.h"

#include "Core/Debug/Debug.h"

#include <glad/glad.h>

namespace Core {
	GraphicsContext::GraphicsContext(GLFWwindow* windowHandle) 
		: m_WindowHandle(windowHandle)
	{
		RC_ASSERT(windowHandle, "Tried to create graphics context with null window handle");
		glfwMakeContextCurrent(windowHandle);

		int32_t success = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		RC_ASSERT(success, "Failed to initialize GLAD");
	}

	void GraphicsContext::SwapBuffers() {
		glfwSwapBuffers(m_WindowHandle);
	}
}