#include "Application.h"

#include "Timestep.h"
#include "Core/Renderer/RenderAPI.h"
#include "Core/Renderer/Renderer2D.h"
#include "Core/Debug/Debug.h"

#include <glfw/glfw3.h>

namespace Core {
    Application* Application::s_Instance = nullptr;

    Application::Application() {
        RC_ASSERT(!s_Instance, "Application already created");
        s_Instance = this;

        m_Window = std::make_unique<Window>(WindowProperties());
        m_Window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

        m_ActiveScene = nullptr;

        RenderAPI::Init();
        Renderer2D::Init();
    }
    
    Application::~Application() {
        for (auto iterator = m_LayerStack.end(); iterator != m_LayerStack.begin();) {
            (*--iterator)->OnDetach();
        }
        m_ActiveScene->Shutdown();
        
        m_Window.reset();

        glfwTerminate();
    }

    void Application::Run() {
        RC_ASSERT(m_ActiveScene, "Active scene not has been set");
        RC_ASSERT(m_LayerStack.begin() != m_LayerStack.end(), "Layer stack is empty");
        while (m_Running)
        {
            float currentFrame = glfwGetTime();
            Timestep deltaTime = currentFrame - m_LastFrame;
            m_LastFrame = currentFrame;

            m_ActiveScene->OnUpdate(deltaTime);
            
            for (auto iterator = m_LayerStack.begin(); iterator != m_LayerStack.end(); iterator++) {
                (*iterator)->OnUpdate(deltaTime);
            }
            m_Window->OnUpdate();
        }
    }

    void Application::OnEvent(Event& event) {
        EventDispatcher dispatcer(event);
        dispatcer.Dispatch<WindowClose>(std::bind(&Application::OnWindowCloseEvent, this, std::placeholders::_1));
        dispatcer.Dispatch<WindowResize>(std::bind(&Application::OnWindowResizeEvent, this, std::placeholders::_1));

        for (auto iterator = m_LayerStack.end(); iterator != m_LayerStack.begin();) {
            if (event.Handled) {
                break;
            }

            (*--iterator)->OnEvent(event);
        }
        
        m_ActiveScene->OnEvent(event);
    }

    void Application::Close() {
        m_Running = false;
    }

    void Application::PushLayer(Layer* layer) {
        RC_ASSERT(layer, "Attempted to push invalid layer");
        m_LayerStack.PushLayer(layer);
        layer->SetScene(m_ActiveScene);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer* overlay) {
        RC_ASSERT(overlay, "Attempted to push invalid overlay");
        m_LayerStack.PushOverlay(overlay);
        overlay->OnAttach();
    }

    void Application::SetActiveScene(Scene* scene) {
        RC_ASSERT(scene, "Attempted to set active scene to nullptr");
        m_ActiveScene = std::shared_ptr<Scene>(scene);
        m_ActiveScene->Init();

        for (auto iterator = m_LayerStack.begin(); iterator != m_LayerStack.end(); iterator++) {
            (*iterator)->SetScene(m_ActiveScene);
        }
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