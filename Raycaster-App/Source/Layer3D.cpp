#include "Layer3D.h"

#include "RaycasterScene.h"

void Layer3D::OnAttach() {
    m_ViewPortWidth = Core::Application::GetWindow().GetWidth() / 2;
    m_ViewPortHeight = Core::Application::GetWindow().GetHeight();
}

void Layer3D::OnUpdate(Core::Timestep deltaTime) {
    RC_ASSERT(dynamic_cast<RaycasterScene*>(m_Scene.get()));
    const RaycasterScene& scene = static_cast<RaycasterScene&>(*m_Scene);

    glm::mat4 viewPerspective = glm::perspective(glm::radians(90.0f), m_ViewPortWidth / (float)m_ViewPortHeight, 5e-4f, 500.0f)
        * scene.GetCamera3D().GetViewMatrix();

    Core::RenderAPI::SetViewPort(m_ViewPortWidth, 0, m_ViewPortWidth, m_ViewPortHeight);
    Core::Renderer::BeginScene(viewPerspective);

    std::span<const Model> models = scene.GetModels();

    for (const Core::Model& model : models) {
        Core::Renderer::DrawModel(model);
    }
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