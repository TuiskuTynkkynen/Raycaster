#include "RaycasterLayer.h"

#include <iostream>
void RaycasterLayer::OnUpdate(Core::Timestep deltaTime) {
    static glm::mat4 identity(1.0f);
    
    std::vector<Core::Ray> rays = m_Scene->GetRays();
    uint32_t rayCount = rays.size();
    glm::vec3 colour;
    
    Core::Renderer2D::SetViewPort(0, 0, 600, 600);

    glm::vec3 rayPos(0.0f);
    glm::vec3 rayScale(2.0f / rayCount, 0.0f, 0.0f);
    glm::vec2 texPos(0.0f);
    glm::vec2 texScale(0.0f, 1.0f);

    Core::Renderer2D::BeginScene(identity);

    for (int i = 0; i < rayCount; i++) {
        rayPos.x = rays[i].rayPosX;
        rayScale.y = rays[i].rayScaleY;
        texPos.x = rays[i].texPosX;

        colour = glm::vec3(1.0f) * rays[i].brightness;

        Core::Renderer2D::DrawTextureQuad(rayPos, rayScale, colour, texPos, texScale);
    }
}


void Layer2D::OnUpdate(Core::Timestep deltaTime) {
    static glm::vec3 AxisZ(0.0f, 0.0f, 1.0f);
    static glm::vec3 zero(0.0f);
    static glm::mat4 identity(1.0f);

    glm::vec3 colour = glm::vec3(0.05f, 0.075f, 0.1f);
    // -------------------- TODO make this update in application --------------------------------------------
    Core::Renderer2D::Clear(colour);
    // -------------------------------------------------------------------------------------------------------

    Core::Renderer2D::SetViewPort(600, 0, 600, 600);
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