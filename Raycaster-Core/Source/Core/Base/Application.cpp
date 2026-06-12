#include "Application.h"

#include "Timestep.h"
#include "Core/Renderer/RenderAPI.h"
#include "Core/Renderer/Renderer2D.h"
#include "Core/Debug/Debug.h"
#include "Platform.h"

#include <GLFW/glfw3.h>

#include <ranges>
#include <utility>

namespace Core {
    Application::Application(const WindowProperties& props) {
        RC_ASSERT(!s_Instance, "Application already created");
        s_Instance = this;

        m_Window = std::make_unique<Window>(props);
        m_Window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

        m_ActiveScene = nullptr;

        RenderAPI::Init();
        Renderer2D::Init();
    }
    
    Application::~Application() {
        m_ActiveScene->OnDetach(*this);
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
        RC_ASSERT(m_ActiveScene, "Active scene not has been set");
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
            
        for (Layer* layer : m_LayerStack.Layers()) {
            RC_ASSERT(layer != nullptr);
            layer->OnUpdate(deltaTime);
        }
        m_Window->OnUpdate();
    }

    void Application::OnEvent(Event& event) {
        EventDispatcher dispatcer(event);
        dispatcer.Dispatch<ApplicationClose>(std::bind(&Application::OnApplicationCloseEvent, this, std::placeholders::_1));
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
        
        m_ActiveScene->OnEvent(event);
    }

    void Application::Close() {
        m_Running = false;
    }

    void Application::PushLayer(std::unique_ptr<Layer> layer) {
        RC_ASSERT(layer, "Attempted to push invalid layer");
        m_LayerStack.PushLayer(std::move(layer), m_ActiveScene);
    }

    void Application::PushOverlay(std::unique_ptr<Layer> overlay) {
        RC_ASSERT(overlay, "Attempted to push invalid overlay");
        m_LayerStack.PushOverlay(std::move(overlay), m_ActiveScene);
    }

    void Application::SetActiveScene(Scene* scene) {
        RC_ASSERT(scene, "Attempted to set active scene to nullptr");
        
        if (m_ActiveScene) { m_ActiveScene->OnDetach(*this); }
        m_LayerCache.Append(std::exchange(m_LayerStack, LayerStack{}));

        m_ActiveScene = std::shared_ptr<Scene>(scene);
        m_ActiveScene->OnAttach(*this);

        for (Layer* layer : m_LayerStack.Layers()) {
            RC_ASSERT(layer != nullptr);
            layer->SetScene(m_ActiveScene);
        }
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
