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

        std::vector<Event*> m_EventQueue;

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
        
        template<typename T, typename... Args> requires std::derived_from<T, Event>
        static inline void PushEvent(Args... args) { s_Instance->m_EventQueue.emplace_back(new T(args...)); }
        
        friend int ::main();
    };

    Application* CreateApplication();
}

