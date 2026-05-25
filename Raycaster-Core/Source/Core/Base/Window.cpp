#include "Window.h"

#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
#include "Core/Events/TextEvent.h"
#include "Core/Events/WindowEvent.h"
#include "Core/Debug/Debug.h"

#include <GLFW/glfw3.h>
#if defined(PLATFORM_EMSCRIPTEN)
    #include <emscripten/html5.h>

    inline static void WebEnterSoftFullscreen(void*) {
        EmscriptenFullscreenStrategy strategy{
            .scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH,
            .canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF,
        };

        if (int32_t result = emscripten_enter_soft_fullscreen("#canvas", &strategy); result < 0) {
            RC_ERROR("Could not set Window to WindowMode::Borderless, entering soft fullscreen failed with error = {}", result);
        }
    }

    inline static void WebEnterHardFullscreen() {
        EmscriptenFullscreenStrategy strategy{
                 .scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH,
                 .canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF,
        };

        if (int32_t result = emscripten_request_fullscreen_strategy("#canvas", EM_TRUE, &strategy); result < 0) {
            RC_ERROR("Could not set Window to WindowMode::Fullscreen, entering hard fullscreen failed with error = {}", result);
        }
    }

    static void WebSwitchWindowMode(Core::WindowMode previous, Core::WindowMode next) {
        using enum Core::WindowMode;
        switch (previous) {
        case Windowed:
            break;
        case Borderless:
            if (int32_t result = emscripten_exit_soft_fullscreen(); result < 0) {
                RC_WARN("Could not exit WindowMode::Borderless, exiting soft fullscreen failed with error = {}", result);
            }
            break;
        case Fullscreen:
            if (int32_t result = emscripten_exit_fullscreen(); result < 0) {
                RC_WARN("Could not exit WindowMode::Fullscreen, exiting hard fullscreen failed with error = {}", result);
            }
            break;
        }

        switch (next) {
        case Windowed:
            return;
        case Borderless:
            return emscripten_async_call(WebEnterSoftFullscreen, nullptr, 1); // Async call to ensure previous WindowMode has exited
        case Fullscreen:
            return WebEnterHardFullscreen();
        }
    }
#endif

namespace Core {
    static bool s_GLFWInitialized = false;

    static void GLFWErrorCallback(int error, const char* desciption) {
        RC_ERROR("GLFW ERROR ({}) {}", error, desciption);
    }

    static void InitGLFW() {
        if (s_GLFWInitialized) {
            return;
        }

        int32_t success = glfwInit();
        RC_ASSERT(success, "Could not initialize GLFW");
        #if !defined(PLATFORM_EMSCRIPTEN)
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        #else
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        #endif

        #ifdef RC_DEBUG_MODE
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
        #endif

        glfwSetErrorCallback(GLFWErrorCallback);

        s_GLFWInitialized = true;
    }

    Window::WindowData Window::CreateWindow(const WindowProperties& properties) {
        InitGLFW();

        if (properties.Mode == WindowMode::Windowed || Platform::WEB) {
            GLFWwindow* window = glfwCreateWindow(properties.Width, properties.Height, properties.Title.c_str(), nullptr, nullptr);
            glm::ivec2 position{};
            glfwGetWindowPos(window, &position.x, &position.y);

            #if defined(PLATFORM_EMSCRIPTEN)
                WebSwitchWindowMode(WindowMode::Windowed, properties.Mode);
            #endif

            return {
                .Window = window,
                .Title = properties.Title,
                .Width = properties.Width,
                .Height = properties.Height,
                .Mode = properties.Mode,
                .WindowedSize = glm::uvec2(properties.Width, properties.Height),
                .WindowedPosition = position,
            };
        }

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
        if (!videoMode) {
            RC_FATAL("Could create a Window with WindowMode::{}, failed to get monitor information", (properties.Mode == WindowMode::Fullscreen) ? "Fullscreen" : "Borderless");
            return { };
        }

        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        auto window = glfwCreateWindow(videoMode->width, videoMode->height, properties.Title.c_str(), (properties.Mode == WindowMode::Fullscreen) ? monitor : nullptr, nullptr);

        int32_t width, height;
        glfwGetWindowSize(window, &width, &height);
        
        glm::ivec2 position{};
        glfwGetMonitorPos(monitor, &position.x, &position.y);
        glm::ivec2 offset = glm::ivec2(videoMode->width - properties.Width, videoMode->height - properties.Height) / 2;
        position += glm::max(offset, glm::ivec2(0, 0));

        return {
              .Window = window,
              .Title = properties.Title,
              .Width = static_cast<uint32_t>(width),
              .Height = static_cast<uint32_t>(height),
              .Mode = properties.Mode,
              .WindowedSize = glm::uvec2(properties.Width, properties.Height),
              .WindowedPosition = position,
        };
    }

    Window::Window(const WindowProperties& properties) {
        m_Data = CreateWindow(properties);
        RC_ASSERT(m_Data.Window, "Failed to create Application Window");
        GLFWwindow* internalWindow = static_cast<GLFWwindow*>(m_Data.Window);
        glfwMakeContextCurrent(internalWindow);
        
        glfwSetWindowUserPointer(internalWindow, &m_Data);
        SetVSync(true);

        glfwSetFramebufferSizeCallback(internalWindow, [](GLFWwindow* window, int width, int height) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            WindowResize event(width, height);
            
            data.Width = width;
            data.Height = height;
            if (data.Mode == WindowMode::Windowed) {
                data.WindowedSize = glm::uvec2(data.Width, data.Height);
            }

            data.EventCallback(event);
        });

        glfwSetWindowPosCallback(internalWindow, [](GLFWwindow* window, int x, int y) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            if (data.Mode == WindowMode::Windowed) {
                data.WindowedPosition = glm::ivec2(x, y);
            }
        });

        glfwSetWindowCloseCallback(internalWindow, [](GLFWwindow* window) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            WindowClose event;
            data.EventCallback(event);
        });

        glfwSetWindowFocusCallback(internalWindow, [](GLFWwindow* window, int32_t focused) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            if (focused == GLFW_TRUE) {
                WindowFocus event;
                data.EventCallback(event);
            } else {
                WindowLostFocus event;
                data.EventCallback(event);
            }
        });

        glfwSetKeyCallback(internalWindow, [](GLFWwindow* window, int key, int scancode, int action, int modifiers){
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action) {
            case GLFW_PRESS: {
                KeyPressed event(key, false);
                data.EventCallback(event);
                }
                break;
            case GLFW_REPEAT: {
                KeyPressed event(key, true);
                data.EventCallback(event);
                }
                break;
            case GLFW_RELEASE: {
                KeyReleased event(key);
                data.EventCallback(event);
                }
                break;
            }
        });

        glfwSetMouseButtonCallback(internalWindow, [](GLFWwindow* window, int button, int action, int mods) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            if (action == GLFW_PRESS) {
                MouseButtonPressed event (button);
                data.EventCallback(event);
            } else {
                MouseButtonReleased event (button);
                data.EventCallback(event);
            }
        });

        glfwSetCursorPosCallback(internalWindow, [](GLFWwindow* window, double posX, double posY) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            MouseMoved event(static_cast<float>(posX), static_cast<float>(posY));
            data.EventCallback(event);
        });

        glfwSetScrollCallback(internalWindow, [](GLFWwindow* window, double offsetX, double offsetY) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            MouseScrolled event(static_cast<float>(offsetX), static_cast<float>(offsetY));
            data.EventCallback(event);
        });

        glfwSetCharCallback(internalWindow, [](GLFWwindow* window, uint32_t codePoint) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            TextInput event(codePoint);
            data.EventCallback(event);
        });
    }

    Window::~Window() {
        glfwDestroyWindow(static_cast<GLFWwindow*>(m_Data.Window));
    }

    void Window::OnUpdate() {
        glfwSwapBuffers(static_cast<GLFWwindow*>(m_Data.Window));

        glfwPollEvents();
    }

    void Window::SetVSync(bool enabled) {
        #if defined(PLATFORM_EMSCRIPTEN)
            RC_WARN("Setting VSync in web builds is not supported");
            return;
        #endif

        if (enabled) {
            glfwSwapInterval(1);
        } else {
            glfwSwapInterval(0);
        }
    }

    static GLFWmonitor* GetWindowMonitor(GLFWwindow* window) {
        glm::ivec2 position{}, size{};
        glfwGetWindowPos(window, &position.x, &position.y);
        glfwGetWindowSize(window, &size.x, &size.y);
        glm::ivec2 windowCenter = position + size / 2;

        int32_t count = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&count);
        for (int32_t i = 0; i < count; i++) {
            glfwGetMonitorWorkarea(monitors[i], &position.x, &position.y, &size.x, &size.y);

            if (glm::all(glm::lessThanEqual(position, windowCenter))
                && glm::all(glm::lessThan(windowCenter, position + size))) {
                return monitors[i];
            }
        }

        return nullptr;
    }

    void Window::SetMode(WindowMode mode) {
        #if defined(PLATFORM_EMSCRIPTEN)
            return WebSwitchWindowMode(m_Data.Mode, (m_Data.Mode = mode));
        #endif
        if (m_Data.Mode == mode) {
            return;
        }            
        m_Data.Mode = mode;

        GLFWwindow* internalWindow = static_cast<GLFWwindow*>(m_Data.Window);
        RC_ASSERT(internalWindow);

        if (mode == WindowMode::Windowed) {
            glfwSetWindowMonitor(internalWindow, nullptr, m_Data.WindowedPosition.x, m_Data.WindowedPosition.y, m_Data.WindowedSize.x, m_Data.WindowedSize.y, 0);
            glfwSetWindowAttrib(internalWindow, GLFW_DECORATED, GLFW_TRUE);

            int32_t top;
            glfwGetWindowFrameSize(internalWindow, nullptr, &top, nullptr, nullptr);
            if (top > m_Data.WindowedPosition.y) {
                glfwSetWindowPos(internalWindow, m_Data.WindowedPosition.x, top);
            }
            return;
        }

        GLFWmonitor* monitor = GetWindowMonitor(internalWindow);
        monitor = monitor ? monitor : glfwGetPrimaryMonitor();

        const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
        if (!videoMode) {
            RC_ERROR("Could not set Window to WindowMode::{}, failed to get monitor information", (mode == WindowMode::Fullscreen) ? "Fullscreen" : "Borderless");
            return;
        }

        int32_t x, y;
        glfwGetMonitorPos(monitor, &x, &y);

        glfwSetWindowAttrib(internalWindow, GLFW_DECORATED, GLFW_FALSE);
        glfwSetWindowMonitor(internalWindow, (mode == WindowMode::Fullscreen) ? monitor : nullptr, x, y, videoMode->width, videoMode->height, videoMode->refreshRate);
    }
}
