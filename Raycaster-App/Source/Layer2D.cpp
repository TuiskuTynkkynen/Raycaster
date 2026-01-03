#include "Layer2D.h"

#include "RaycasterScene.h"

void Layer2D::OnAttach() {
    m_ViewPortWidth = Core::Application::GetWindow().GetWidth() / 2;
    m_ViewPortHeight = Core::Application::GetWindow().GetHeight();
}

void Layer2D::OnUpdate(Core::Timestep deltaTime) {
    RC_ASSERT(dynamic_cast<RaycasterScene*>(m_Scene.get()));
    const RaycasterScene& scene = static_cast<RaycasterScene&>(*m_Scene);
    constexpr glm::vec3 AxisZ(0.0f, 0.0f, 1.0f);

    Core::RenderAPI::SetViewPort(m_ViewPortWidth, 0, m_ViewPortWidth, m_ViewPortHeight);
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
    Core::Renderer2D::DrawRotatedFlatQuad(player.GetPosition(), -player.GetRotation(), AxisZ, player.GetScale(), colour);

    Core::Renderer2D::Flush();

    colour = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    std::span<const Line> lines = scene.GetLines();
    size_t lineCount = lines.size();
    for (size_t i = 0; i < lineCount; i++) {
        Core::Renderer2D::DrawLine(lines[i].Posistion, lines[i].Scale, colour);
    }

    Core::Renderer2D::EndScene();
}

void Layer2D::OnEvent(Core::Event& event) {
    Core::EventDispatcher dispatcer(event);
    dispatcer.Dispatch<Core::WindowResize>(std::bind(&Layer2D::OnWindowResizeEvent, this, std::placeholders::_1));
}

bool Layer2D::OnWindowResizeEvent(Core::WindowResize& event) {
    m_ViewPortWidth = event.GetWidth() / 2;
    m_ViewPortHeight = event.GetHeight();

    return false;
}