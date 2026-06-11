#include "RaycasterLayer.h"

#include "RaycasterScene.h"
#include "Settings/Video.h"

void RaycasterLayer::OnAttach() {}
void RaycasterLayer::OnDetach() {}

void RaycasterLayer::OnUpdate(Core::Timestep deltaTime) {
    const glm::uvec2 viewSize = Settings::Video::ViewPortSize(Settings::Video::LayerType::Raycaster);
    if (viewSize.x * viewSize.y == 0) {
        return;
    }

    const auto lock = m_Scene.lock();
    RC_ASSERT(dynamic_cast<const RaycasterScene*>(lock.get()));
    const RaycasterScene& scene = static_cast<const RaycasterScene&>(*lock);
    constexpr glm::mat4 identity(1.0f);

    const uint32_t viewOffset = Settings::Video::ViewPortOffset(Settings::Video::LayerType::Raycaster);
    Core::RenderAPI::SetViewPort(viewOffset, 0, viewSize.x, viewSize.y);
    Core::Renderer2D::BeginScene(identity);

    glm::vec4 colour, colour1;
    glm::vec3 rayScale(0.0f, RaycastRenderer::GetRayWidth(), 0.0f);
    glm::vec2 texScale(0.0f);

    const auto& floors = scene.GetFloors();
    float rot = -scene.GetPlayer().GetYaw() + 90.0f;
    for (const auto& ray : floors) {
        glm::vec3 rayPos = glm::vec3(ray.Position, 0.0f);
        rayScale.x = ray.Length;
        texScale.x = ray.Length * 0.5f * ray.Scale;

        colour = glm::vec4(ray.BrightnessEnd);
        colour.a = 1.0f;

        colour1 = glm::vec4(ray.BrightnessStart);
        colour1.a = 1.0f;

        Core::Renderer2D::DrawTextureGradientQuad(rayPos, rayScale, colour, colour1, ray.TexturePosition, texScale, ray.AtlasIndex, rot);
    }

    rayScale.x = RaycastRenderer::GetRayWidth();
    texScale = glm::vec2(0.0f, 1.0f);

    std::span<const Ray> rays = scene.GetRays();
    for (const auto& ray : rays) {
        glm::vec3 rayPos = glm::vec3(ray.Position, 0.0f);
        rayScale.y = ray.Scale;

        colour = glm::vec4(ray.Brightness);
        colour.a = 1.0f;

        Core::Renderer2D::DrawTextureQuad(rayPos, rayScale, colour, ray.TexPosition, texScale, ray.Atlasindex);
    }

    Core::Renderer2D::EndScene();
}