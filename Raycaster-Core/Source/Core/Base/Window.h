#pragma once

#include "Core/Events/Event.h"

#include <glm/glm.hpp>

#include <string>
#include <functional>
#include <cstdint>

namespace Core {
    // WindowMode support is best effort and may differ based on platform.
    enum class WindowMode : uint8_t {
        Windowed = 0, // Resizable Window with decorations.
        Borderless,   // Window with no decorations and size = monitor size.
        Fullscreen,   // True fullscreen Window.
    };

    struct WindowProperties {
        std::string Title = "Application";
        WindowMode Mode = WindowMode::Windowed;
        uint32_t Width = 1200;
        uint32_t Height = 600;
    };

    class Window {
        using EventCallbackFunction = std::function<void(Event&)>;
    public:
        Window(const WindowProperties& properties);
        ~Window();

        void OnUpdate();

        uint32_t GetHeight() const { return m_Data.Height;  }
        uint32_t GetWidth() const { return m_Data.Width; }
        
        inline void SetEventCallback(const EventCallbackFunction& callback) { m_Data.EventCallback = callback; }
        void SetVSync(bool enbled);
        void SetMode(WindowMode mode);

        inline void* GetWindowPointer() { return m_Data.Window;  }
    private:
        struct WindowData {
            void* Window = nullptr;

            std::string Title;
            uint32_t Width = 0;
            uint32_t Height = 0;

            WindowMode Mode = WindowMode::Windowed;
            glm::uvec2 WindowedSize = {};
            glm::ivec2 WindowedPosition = {};

            EventCallbackFunction EventCallback;
        };
        WindowData m_Data;

        static WindowData CreateWindow(const WindowProperties& properties);
    };
}
