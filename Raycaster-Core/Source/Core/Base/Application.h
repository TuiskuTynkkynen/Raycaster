#pragma once

#include "Window.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/WindowEvent.h"
#include "LayerStack.h"
#include "Core/Scene/Scene.h"

#include <memory>

int main();

namespace Core {
    class Application
    {
    private:
        static Application* s_Instance;

        std::unique_ptr<Window> m_Window;
        std::shared_ptr<Scene> m_ActiveScene;
        LayerStack m_LayerStack;

        bool m_Running = true;
        float m_LastFrame = 0.0f;

        bool OnApplicationCloseEvent(ApplicationClose& event);
        bool OnWindowCloseEvent(WindowClose& event);
        bool OnWindowResizeEvent(WindowResize& event);

        void Run();
    public:
        Application();
        virtual ~Application();
        
        void OnEvent(Event& event);
        void Close();

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);
        
        void SetActiveScene(Scene* scene);

        static inline Window& GetWindow() { return *s_Instance->m_Window; }

        friend int ::main();
    };

    Application* CreateApplication();
}

