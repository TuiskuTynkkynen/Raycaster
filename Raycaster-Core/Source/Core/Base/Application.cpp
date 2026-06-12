#include "Application.h"

#include "Timestep.h"
#include "Core/Renderer/RenderAPI.h"
#include "Core/Renderer/Renderer2D.h"
#include "Core/Debug/Debug.h"
#include "Platform.h"

#include <GLFW/glfw3.h>

#include <ranges>

namespace Core {
    Application::Application(const WindowProperties& props) {
        RC_ASSERT(!s_Instance, "Application already created");
        s_Instance = this;

        m_Window = std::make_unique<Window>(props);
        m_Window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

        RenderAPI::Init();
        Renderer2D::Init();
    }
    
    Application::~Application() {
        for (auto& scene : m_SceneStack) {
            scene->OnDetach(*this);
        }
        m_LayerStack.Clear();

        m_Window.reset();

        glfwTerminate();
    }

#if defined(PLATFORM_EMSCRIPTEN)
    #include <emscripten/emscripten.h>

    void Application::Run() {
        auto mainLoop = [](void* application) {
            Application* app = (Application*)application;
            if(!app->m_Running) { emscripten_cancel_main_loop(); }
            app->Update();
        };
        emscripten_set_main_loop_arg(mainLoop, this, 0, true);
    }
#else
    void Application::Run() {
        while (m_Running) {
            Update();
        }
    }
#endif

    void Application::Update() {
        RC_ASSERT(!m_SceneStack.empty(), "Scene stack is empty");
        RC_ASSERT(m_LayerStack.Size(), "Layer stack is empty");

        float currentFrame = static_cast<float>(glfwGetTime());
        Timestep deltaTime = currentFrame - m_LastFrame;
        m_LastFrame = currentFrame;

        for (size_t i = 0; i < m_EventQueue.size(); i++) {
            OnEvent(*m_EventQueue[i]);
            delete m_EventQueue[i];
        }
        m_EventQueue.clear();
        
        if (!m_Running) { 
            return; // Recieved Application Close event
        }

        m_SceneStack.back()->OnUpdate(deltaTime);
            
        for (Layer* layer : m_LayerStack.Layers()) {
            RC_ASSERT(layer != nullptr);
            layer->OnUpdate(deltaTime);
        }
        m_Window->OnUpdate();
    }

    void Application::OnEvent(Event& event) {
        EventDispatcher dispatcer(event);
        dispatcer.Dispatch<ApplicationClose>(std::bind(&Application::OnApplicationCloseEvent, this, std::placeholders::_1));
        dispatcer.Dispatch<ApplicationScenePop>(std::bind(&Application::OnApplicationScenePopEvent, this, std::placeholders::_1));
        dispatcer.Dispatch<WindowClose>(std::bind(&Application::OnWindowCloseEvent, this, std::placeholders::_1));
        dispatcer.Dispatch<WindowResize>(std::bind(&Application::OnWindowResizeEvent, this, std::placeholders::_1));

        if (!m_Running || event.Handled) { return; }

        for (Layer* layer : m_LayerStack.Layers() | std::ranges::views::reverse) {
            RC_ASSERT(layer != nullptr);
            if (event.Handled) {
                break;
            }

            layer->OnEvent(event);
        }
        
        RC_ASSERT(!m_SceneStack.empty());
        m_SceneStack.back()->OnEvent(event);
    }

    void Application::Close() {
        m_Running = false;
    }

    void Application::PushLayer(std::unique_ptr<Layer> layer) {
        RC_ASSERT(layer, "Attempted to push invalid layer");
        m_LayerStack.PushLayer(std::move(layer), GetActiveScene());
    }

    void Application::PushOverlay(std::unique_ptr<Layer> overlay) {
        RC_ASSERT(overlay, "Attempted to push invalid overlay");
        m_LayerStack.PushOverlay(std::move(overlay), GetActiveScene());
    }

    void Application::PushScene(std::shared_ptr<Scene> scene) {
        if (!m_SceneStack.empty()) { 
            m_SceneStack.back()->OnDetach(*this); 
        }

        RC_ASSERT(scene);
        m_SceneStack.push_back(scene);
        AttachScene(scene);
    }

    void Application::PopScene() {
        RC_ASSERT(s_Instance);
        RC_ASSERT(!s_Instance->m_SceneStack.empty(), "Tried to pop more Scenes than have been pushed");

        s_Instance->PushEvent<ApplicationScenePop>();
        if (s_Instance->m_SceneStack.size() <= 1) {
            s_Instance->PushEvent<ApplicationClose>();
        }
    }

    void Application::AttachScene(std::weak_ptr<Scene> scene) {
        m_LayerCache.Append(std::exchange(m_LayerStack, LayerStack{}));

        if (auto lock = scene.lock()) { 
            lock->OnAttach(*this);
        }
        
        for (Layer* layer : m_LayerStack.Layers()) {
            RC_ASSERT(layer != nullptr);
            layer->SetScene(scene);
        }
    }

    bool Application::OnApplicationScenePopEvent(ApplicationScenePop& event) {
        RC_ASSERT(!m_SceneStack.empty());

        m_SceneStack.back()->OnDetach(*this);
        m_SceneStack.pop_back();
        AttachScene(s_Instance->GetActiveScene());
        return true;
    }

    bool Application::OnApplicationCloseEvent(ApplicationClose& event) {
        m_Running = false;
        return true;
    }

    bool Application::OnWindowCloseEvent(WindowClose& event) {
        m_Running = false;
        return true;
    }

    bool Application::OnWindowResizeEvent(WindowResize& event) {
        RenderAPI::SetViewPort(0, 0, event.GetWidth(), event.GetHeight());
        return false;
    }
}
