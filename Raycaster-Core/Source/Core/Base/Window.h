#pragma once

#include "Core/Events/Event.h"

#include <string>
#include <functional>
#include <cstdint>

namespace Core {
    struct WindowProperties {
        std::string Tittle;
        uint32_t Width;
        uint32_t Height;

        WindowProperties(const std::string& tittle = "Application", uint32_t width = 1200, uint32_t height = 600)
            : Tittle(tittle), Width(width), Height(height) {}
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

        inline void* GetWindowPointer() { return m_Window;  }
    private:
        void* m_Window;

        struct WindowData {
            std::string Tittle;
            uint32_t Width = 0;
            uint32_t Height = 0;

            bool VSyncEnabled = false;
            EventCallbackFunction EventCallback;
        };

        WindowData m_Data;
    };
}
