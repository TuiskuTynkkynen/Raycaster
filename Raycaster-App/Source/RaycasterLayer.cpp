#include "RaycasterLayer.h"

#include <iostream>
void RaycasterLayer::OnUpdate(Core::Timestep deltaTime) {
    static glm::mat4 identity(1.0f);
    
    Core::Renderer2D::SetViewPort(0, 0, m_ViewPortWidth, m_ViewPortHeight);
    Core::Renderer2D::BeginScene(identity);
    
    std::vector<Core::Ray> rays = m_Scene->GetRays();
    uint32_t rayCount = m_Scene->GetRayCount();
    uint32_t rayArraySize = rays.size();
    glm::vec3 colour;
    
    glm::vec3 rayPos(0.0f);
    glm::vec3 rayScale(2.0f, 2.0f / rayCount, 0.0f);
    glm::vec2 texScale(0.0f, 0.0f);

    for (int i = rayCount; i < rayCount * 2; i++) {
        rayPos.y = rays[i].Position.y;
        texScale.x = rays[i].Scale;

        colour = glm::vec3(1.0f) * rays[i].Brightness;

        Core::Renderer2D::DrawTextureQuad(rayPos, rayScale, colour, rays[i].TexPosition, texScale, rays[i].Atlasindex, rays[i].TexRotation);
    }

    rayPos.y = 0.0f;
    rayScale.x = 2.0f / rayCount;
    texScale = glm::vec2(0.0f, 1.0f);

    for (int i = 0; i < rayCount; i++) {
        rayPos.x = rays[i].Position.x;
        rayScale.y = rays[i].Scale;
        
        colour = glm::vec3(1.0f) * rays[i].Brightness;

        Core::Renderer2D::DrawTextureQuad(rayPos, rayScale, colour, rays[i].TexPosition, texScale, rays[i].Atlasindex);
    }

    for (int i = 2 * rayCount; i < rayArraySize; i++) {
        rayPos.x = rays[i].Position.x;
        rayPos.y = rays[i].Position.y;
        rayScale.y = rays[i].Scale;

        colour = glm::vec3(1.0f) * rays[i].Brightness;

        Core::Renderer2D::DrawTextureQuad(rayPos, rayScale, colour, rays[i].TexPosition, texScale, rays[i].Atlasindex);
    }
}

void RaycasterLayer::OnEvent(Core::Event& event) {
    Core::EventDispatcher dispatcer(event);
    dispatcer.Dispatch<Core::WindowResize>(std::bind(&RaycasterLayer::OnWindowResizeEvent, this, std::placeholders::_1));
}

bool RaycasterLayer::OnWindowResizeEvent(Core::WindowResize& event) {
    m_ViewPortWidth = event.GetWidth() * 0.5f;
    m_ViewPortHeight = event.GetHeight();

    return false;
}

void Layer2D::OnUpdate(Core::Timestep deltaTime) {
    static glm::vec3 AxisZ(0.0f, 0.0f, 1.0f);
    static glm::vec3 zero(0.0f);
    static glm::mat4 identity(1.0f);

    glm::vec3 colour = glm::vec3(0.05f, 0.075f, 0.1f);
    
    Core::Renderer2D::SetViewPort(m_ViewPortWidth, 0, m_ViewPortWidth, m_ViewPortHeight);
    Core::Renderer2D::BeginScene(m_Scene->GetCamera());

    std::vector<Core::FlatQuad> tiles = m_Scene->GetQuads();
    uint32_t mapSize = tiles.size();

    for (int i = 0; i < mapSize; i++) {
        Core::Renderer2D::DrawFlatQuad(tiles[i].Posistion, tiles[i].Scale, tiles[i].Colour);
    }

    Core::Renderer2D::BeginScene(identity);

    colour = glm::vec3(1.0f, 0.0f, 0.0f);
    Core::Player& player = m_Scene->GetPlayer();
    Core::Renderer2D::DrawRotatedFlatQuad(zero, player.Rotation, AxisZ, player.Scale, colour);

    colour = glm::vec3(0.0f, 0.0f, 1.0f);
    std::vector <Core::Line> lines = m_Scene->GetLines();
    uint32_t lineCount = lines.size();
    for (int i = 0; i < lineCount; i++) {
        Core::Renderer2D::DrawLine(lines[i].Posistion, lines[i].Scale, colour);
    }
}

void Layer2D::OnEvent(Core::Event& event) {
    Core::EventDispatcher dispatcer(event);
    dispatcer.Dispatch<Core::WindowResize>(std::bind(&Layer2D::OnWindowResizeEvent, this, std::placeholders::_1));
}

bool Layer2D::OnWindowResizeEvent(Core::WindowResize& event) {
    m_ViewPortWidth = event.GetWidth() * 0.5f;
    m_ViewPortHeight = event.GetHeight();

    return false;
}
