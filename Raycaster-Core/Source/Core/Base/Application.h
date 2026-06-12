#pragma once

#include "Window.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/WindowEvent.h"
#include "LayerStack.h"
#include "Core/Scene/Scene.h"

#include <memory>
#include <typeinfo>

int main();

namespace Core {
    class Application {
    public:
        Application(const WindowProperties& properties = {});
        virtual ~Application();
        
        void OnEvent(Event& event);
        void Close();

        template<typename T>
        void RequestLayer() requires std::derived_from<T, Layer> {
            if (m_LayerStack.PushCachedLayer(typeid(T), m_LayerCache, m_ActiveScene)) { return; }
            m_LayerStack.PushLayer(std::make_unique<T>(), m_ActiveScene);
        }
        template<typename T>
        void RequestOverlay() requires std::derived_from<T, Layer> {
            if (m_LayerStack.PushCachedOverlay(typeid(T), m_LayerCache, m_ActiveScene)) { return; }
            m_LayerStack.PushOverlay(std::make_unique<T>(), m_ActiveScene);
        }

        void SetActiveScene(Scene* scene);
        
        static inline Window& GetWindow() { return *s_Instance->m_Window; }
        
        template<typename T, typename... Args> requires std::derived_from<T, Event>
        static inline void PushEvent(Args... args) { s_Instance->m_EventQueue.emplace_back(new T(args...)); }
        void PushLayer(std::unique_ptr<Layer> layer);       // Prefer RequestLayer
        void PushOverlay(std::unique_ptr<Layer> overlay);   // Prefer RequestOverlay
    private:
        inline static Application* s_Instance = nullptr;

        std::unique_ptr<Window> m_Window;
        std::shared_ptr<Scene> m_ActiveScene;
        LayerStack m_LayerStack;
        LayerCache m_LayerCache;

        std::vector<Event*> m_EventQueue;

        bool m_Running = true;
        float m_LastFrame = 0.0f;

        bool OnApplicationCloseEvent(ApplicationClose& event);
        bool OnWindowCloseEvent(WindowClose& event);
        bool OnWindowResizeEvent(WindowResize& event);

        void Run();
        void Update();
        friend int ::main();
    };

    Application* CreateApplication();
}

