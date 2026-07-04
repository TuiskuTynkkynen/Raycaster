#include "Layer3D.h"

#include "RaycasterScene.h"
#include "Settings/Video.h"

#include "Core/Renderer/RenderAPI.h"
#include "Core/Renderer/Renderer.h"

void Layer3D::OnAttach() {
    m_Framebuffer.InitRender();
    m_PostProcessShader = std::make_unique<Core::Shader>("Assets/Shaders/PostProcess.glsl");
}

void Layer3D::OnDetach() {
    m_Framebuffer.ShutdownRender();
};

void Layer3D::OnUpdate(Core::Timestep deltaTime) {
    const glm::uvec2 viewSize = Settings::Video::ViewPortSize(Settings::Video::LayerType::Layer3D);
    if (viewSize.x * viewSize.y == 0) {
        return;
    }

    const auto lock = m_Scene.lock();
    RC_ASSERT(dynamic_cast<const RaycasterScene*>(lock.get()));
    const RaycasterScene& scene = static_cast<const RaycasterScene&>(*lock);
    m_Framebuffer.Activate();
    Core::RenderAPI::Clear();
    Core::RenderAPI::SetViewPort(0, 0, 500, 500);
    
    glm::mat4 viewPerspective = glm::perspective(glm::radians(90.0f), viewSize.x / (float)viewSize.y, 5e-4f, 500.0f)
        * scene.GetCamera3D().GetViewMatrix();
    
    Core::Renderer::BeginScene(viewPerspective);
    
    std::span<const Model> models = scene.GetModels();
    for (const Core::Model& model : models) {
        Core::Renderer::DrawModel(model);
    }
    
    m_Framebuffer.Deactivate();
    
    const uint32_t viewOffset = Settings::Video::ViewPortOffset(Settings::Video::LayerType::Layer3D);
    Core::RenderAPI::SetViewPort(viewOffset, 0, viewSize.x, viewSize.y);
    m_Framebuffer.Render(*m_PostProcessShader, 0);
}