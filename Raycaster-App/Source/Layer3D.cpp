#include "Layer3D.h"

#include "RaycasterScene.h"

void Layer3D::OnAttach() {
    m_ViewPortWidth = Core::Application::GetWindow().GetWidth() / 2;
    m_ViewPortHeight = Core::Application::GetWindow().GetHeight();

    m_Framebuffer.InitRender();
    m_PostProcessShader = std::make_unique<Core::Shader>("Assets/Shaders/PostProcess.glsl");
}

void Layer3D::OnDetach() {
    m_Framebuffer.ShutdownRender();
};

void Layer3D::OnUpdate(Core::Timestep deltaTime) {
    RC_ASSERT(dynamic_cast<RaycasterScene*>(m_Scene.get()));
    const RaycasterScene& scene = static_cast<RaycasterScene&>(*m_Scene);
    
    m_Framebuffer.Activate();
    Core::RenderAPI::Clear();
    Core::RenderAPI::SetViewPort(0, 0, 500, 500);
    
    glm::mat4 viewPerspective = glm::perspective(glm::radians(90.0f), m_ViewPortWidth / (float)m_ViewPortHeight, 5e-4f, 500.0f)
        * scene.GetCamera3D().GetViewMatrix();
    
    Core::Renderer::BeginScene(viewPerspective);
    
    std::span<const Model> models = scene.GetModels();
    for (const Core::Model& model : models) {
        Core::Renderer::DrawModel(model);
    }
    
    m_Framebuffer.Deactivate();
    
    Core::RenderAPI::SetViewPort(m_ViewPortWidth, 0, m_ViewPortWidth, m_ViewPortHeight);
    m_Framebuffer.Render(*m_PostProcessShader, 0);
}

void Layer3D::OnEvent(Core::Event& event) {
    Core::EventDispatcher dispatcer(event);
    dispatcer.Dispatch<Core::WindowResize>(std::bind(&Layer3D::OnWindowResizeEvent, this, std::placeholders::_1));
}

bool Layer3D::OnWindowResizeEvent(Core::WindowResize& event) {
    m_ViewPortWidth = event.GetWidth() / 2;
    m_ViewPortHeight = event.GetHeight();

    return false;
}