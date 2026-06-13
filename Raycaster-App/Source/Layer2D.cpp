#include "Layer2D.h"

#include "RaycasterScene.h"
#include "Settings/Video.h"

#include "Core/Renderer/RenderAPI.h"
#include "Core/Renderer/Renderer2D.h"

void Layer2D::OnAttach() {}

void Layer2D::OnUpdate(Core::Timestep deltaTime) {
    const glm::uvec2 viewSize = Settings::Video::ViewPortSize(Settings::Video::LayerType::Layer2D);
    if (viewSize.x * viewSize.y == 0) {
        return;
    }

    auto lock = m_Scene.lock();
    RC_ASSERT(dynamic_cast<const RaycasterScene*>(lock.get()) != nullptr);
    const RaycasterScene& scene = static_cast<const RaycasterScene&>(*lock);
    constexpr glm::vec3 AxisZ(0.0f, 0.0f, 1.0f);

    const uint32_t viewOffset = Settings::Video::ViewPortOffset(Settings::Video::LayerType::Layer2D);
    Core::RenderAPI::SetViewPort(viewOffset, 0, viewSize.x, viewSize.y);
    Core::Renderer2D::BeginScene(scene.GetCamera());

    std::span<const Tile> tiles = scene.GetTiles();
    size_t mapSize = tiles.size();

    for (size_t i = 0; i < mapSize; i++) {
        if (tiles[i].IsTriangle) {
            Core::Renderer2D::DrawRotatedFlatTriangle(tiles[i].Posistion, tiles[i].Rotation, AxisZ, tiles[i].Scale, { tiles[i].Colour.x, tiles[i].Colour.y, tiles[i].Colour.z, 1.0f });
        } else {
            Core::Renderer2D::DrawFlatQuad(tiles[i].Posistion, tiles[i].Scale, { tiles[i].Colour.x, tiles[i].Colour.y, tiles[i].Colour.z, 1.0f });
        }
    }

    glm::vec4 colour = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    const Player& player = scene.GetPlayer();
    Core::Renderer2D::DrawRotatedFlatQuad(player.GetPosition(), -player.GetYaw(), AxisZ, player.GetScale(), colour);

    Core::Renderer2D::Flush();

    colour = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    std::span<const Line> lines = scene.GetLines();
    size_t lineCount = lines.size();
    for (size_t i = 0; i < lineCount; i++) {
        Core::Renderer2D::DrawLine(lines[i].Posistion, lines[i].Scale, colour);
    }

    Core::Renderer2D::EndScene();
}