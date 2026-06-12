#pragma once

#include "Window.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/WindowEvent.h"
#include "LayerStack.h"
#include "Core/Scene/Scene.h"

#include <memory>
#include <utility>
#include <typeinfo>

int main();

namespace Core {
    class Application {
    public:
        Application(const WindowProperties& properties = {});
        virtual ~Application();
        
        void OnEvent(Event& event);
        void Close();

        template<typename T, typename... Args>
        static void PushScene(Args... args) { s_Instance->PushScene(std::make_shared<T>(std::forward<Args>(args)...)); }
        static void PopScene();

        template<typename T>
        void RequestLayer() requires std::derived_from<T, Layer> {
            if (m_LayerStack.PushCachedLayer(typeid(T), m_LayerCache, GetActiveScene())) { return; }
            m_LayerStack.PushLayer(std::make_unique<T>(), GetActiveScene());
        }
        template<typename T>
        void RequestOverlay() requires std::derived_from<T, Layer> {
            if (m_LayerStack.PushCachedOverlay(typeid(T), m_LayerCache, GetActiveScene())) { return; }
            m_LayerStack.PushOverlay(std::make_unique<T>(), GetActiveScene());
        }

        static inline Window& GetWindow() { return *s_Instance->m_Window; }
        static inline std::weak_ptr<Scene> GetActiveScene() { return !s_Instance->m_SceneStack.empty() ? s_Instance->m_SceneStack.back() : std::weak_ptr<Scene>{}; }
        
        template<typename T, typename... Args> requires std::derived_from<T, Event>
        static inline void PushEvent(Args... args) { s_Instance->m_EventQueue.emplace_back(new T(args...)); }
        void PushLayer(std::unique_ptr<Layer> layer);       // Prefer RequestLayer
        void PushOverlay(std::unique_ptr<Layer> overlay);   // Prefer RequestOverlay
    private:
        inline static Application* s_Instance = nullptr;

        std::unique_ptr<Window> m_Window;
        std::vector<std::shared_ptr<Scene>> m_SceneStack;
        LayerStack m_LayerStack;
        LayerCache m_LayerCache;

        std::vector<Event*> m_EventQueue;

        bool m_Running = true;
        float m_LastFrame = 0.0f;

        bool OnApplicationCloseEvent(ApplicationClose& event);
        bool OnApplicationScenePopEvent(ApplicationScenePop& event);
        bool OnWindowCloseEvent(WindowClose& event);
        bool OnWindowResizeEvent(WindowResize& event);

        void Run();
        void Update();
        void PushScene(std::shared_ptr<Scene> newActiveScene);
        void AttachScene(std::weak_ptr<Scene> newActiveScene);
        friend int ::main();
    };

    Application* CreateApplication();
}

