#include "RaycasterLayer.h"

#include "RaycasterScene.h"

void RaycasterLayer::OnAttach() {}
void RaycasterLayer::OnDetach() {}

void RaycasterLayer::OnUpdate(Core::Timestep deltaTime) { 
    static glm::mat4 identity(1.0f);

    Core::RenderAPI::SetViewPort(0, 0, m_ViewPortWidth, m_ViewPortHeight);
    Core::Renderer2D::BeginScene(identity);

    glm::vec4 colour, colour1;
    glm::vec3 rayPos(0.0f);
    glm::vec3 rayScale(0.0f, RaycastRenderer::GetRayWidth(), 0.0f);
    glm::vec2 texScale(0.0f);

    const auto& floors = static_cast<RaycasterScene&>(*m_Scene).GetFloors();
    float rot = -static_cast<RaycasterScene&>(*m_Scene).GetPlayer().GetRotation() + 90.0f;

    for (const auto& ray : floors) {
        rayPos.x = ray.Position.x;
        rayPos.y = ray.Position.y;

        rayScale.x = ray.Length;
        texScale.x = ray.Length * (m_ViewPortWidth / (2.0f * m_ViewPortHeight * ray.Position.y));

        colour = glm::vec4(ray.BrightnessEnd);
        colour.a = 1.0f;

        colour1 = glm::vec4(ray.BrightnessStart);
        colour1.a = 1.0f;

        Core::Renderer2D::DrawTextureGradientQuad(rayPos, rayScale, colour, colour1, ray.TexturePosition, texScale, ray.TopAtlasIndex, rot);

        rayPos.y *= -1.0f;
        Core::Renderer2D::DrawTextureGradientQuad(rayPos, rayScale, colour, colour1, ray.TexturePosition, texScale, ray.BottomAtlasIndex, rot);
    }

    rayScale.x = RaycastRenderer::GetRayWidth();
    texScale = glm::vec2(0.0f, 1.0f);

    std::span<const Ray> rays = static_cast<RaycasterScene&>(*m_Scene).GetRays();
    for (const auto& ray : rays) {
        rayPos.x = ray.Position.x;
        rayPos.y = ray.Position.y;
        rayScale.y = ray.Scale;

        colour = glm::vec4(ray.Brightness);
        colour.a = 1.0f;

        Core::Renderer2D::DrawTextureQuad(rayPos, rayScale, colour, ray.TexPosition, texScale, ray.Atlasindex);
    }

    Core::Renderer2D::EndScene();
}

void RaycasterLayer::OnEvent(Core::Event& event) {
    Core::EventDispatcher dispatcer(event);
    dispatcer.Dispatch<Core::WindowResize>(std::bind(&RaycasterLayer::OnWindowResizeEvent, this, std::placeholders::_1));
}

bool RaycasterLayer::OnWindowResizeEvent(Core::WindowResize& event) {
    m_ViewPortWidth = event.GetWidth() / 2;
    m_ViewPortHeight = event.GetHeight();

    return false;
}