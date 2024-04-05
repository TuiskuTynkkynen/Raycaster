#include "RaycasterScene.h"

#include <iostream>

RaycasterScene::MapData RaycasterScene::s_MapData;

void RaycasterScene::Init(){
    m_Rays.resize(m_RayCount); //should be initialized to default values
    m_Lines.resize(m_RayCount); //should be initialized to default vec3s

    m_Sprites.resize(m_RayCount); //should be initialized to default vec3s
    {
        glm::vec3 rayScale(2.0f, 2.0f / m_RayCount, 0.0f);
        glm::vec3 colour(0.8f);
        uint32_t Index = 7; 

        for (uint32_t i = 0; i < m_RayCount; i++)
        {
            float cameraY = 2 * i / float(m_RayCount) - 1;
            m_Sprites[i].Posistion.y = cameraY;
            m_Sprites[i].Scale = rayScale;
            m_Sprites[i].Atlasindex = Index;

            if (i == m_RayCount * 0.5f) { colour = glm::vec3(1.0f); Index = 6; }
            m_Sprites[i].Colour = colour;
        }
    }

    m_Player.Position = glm::vec3((float)s_MapData.width / 2, (float)s_MapData.heigth / 2, 1.0f);
    m_Player.Scale = s_MapData.mapScale * 0.4f;
    m_Player.Rotation = 90.0f;

    m_Camera = std::make_unique<Core::RaycasterCamera>(m_Player.Position, m_Player.Rotation, s_MapData.mapScalingFactor, s_MapData.width, s_MapData.heigth);
    
    Core::FlatQuad tile;
    tile.Scale = glm::vec3(0.95f / s_MapData.mapScalingFactor, 0.95f / s_MapData.mapScalingFactor, 1.0f);
    tile.Posistion.z = 0.0f;

    float centreY = (float)(s_MapData.heigth - 1) / 2, centreX = (float)(s_MapData.width - 1) / 2;
    for (uint32_t i = 0; i < s_MapData.size; i++) {
        uint32_t mapX = i % s_MapData.width;
        uint32_t mapY = i / s_MapData.width;

        tile.Posistion.x = (mapX - centreX) / (s_MapData.mapScalingFactor);
        tile.Posistion.y = (centreY - mapY) / (s_MapData.mapScalingFactor);
        
        float brightness = (s_MapData.map[mapY * s_MapData.width + mapX] != 0) ? 1.0f : 0.5f;

        tile.Colour = glm::vec3(brightness);
        m_Tiles.push_back(tile);
    }
}

void RaycasterScene::OnUpdate(Core::Timestep deltaTime) {
    if (!m_Paused) {
        glm::vec3 colour = glm::vec3(0.05f, 0.075f, 0.1f);
        Core::Renderer2D::Clear(colour);
        ProcessInput(deltaTime);
        CastRays();
    }
}

void RaycasterScene::CastRays() {
    //Wall casting
    for (uint32_t i = 0; i < m_RayCount; i++)
    {
        float cameraX = 2 * i / float(m_RayCount) - 1;
        glm::vec3 rayDirection = m_Camera->direction + m_Camera->plane * cameraX;
        glm::vec3 deltaDistance = glm::abs((float)1 / rayDirection);

        uint32_t mapX = (int)m_Player.Position.x;
        uint32_t mapY = (int)m_Player.Position.y;

        int32_t stepX = (rayDirection.x > 0) ? 1 : -1;
        int32_t stepY = (rayDirection.y < 0) ? 1 : -1;

        glm::vec3 sideDistance = deltaDistance;
        sideDistance.x *= (rayDirection.x < 0) ? (m_Player.Position.x - mapX) : (mapX + 1.0f - m_Player.Position.x);
        sideDistance.y *= (rayDirection.y > 0) ? (m_Player.Position.y - mapY) : (mapY + 1.0f - m_Player.Position.y);

        uint32_t hit = 0;
        uint32_t side = 0;

        while (hit == 0)
        {
            if (sideDistance.x < sideDistance.y) {
                sideDistance.x += deltaDistance.x;
                mapX += stepX;
                side = 0;
            }
            else {
                sideDistance.y += deltaDistance.y;
                mapY += stepY;
                side = 1;
            }

            if (mapY >= s_MapData.heigth || mapX >= s_MapData.width) {
                std::cout << "ERROR: INDEX OUT OF BOUNDS" << std::endl;
                break;
            }
            if (s_MapData.map[mapY * s_MapData.width + mapX] > 0) {
                hit = 1;
            }
        }

        float wallDistance;
        if (side == 0) {
            wallDistance = sideDistance.x - deltaDistance.x;
            m_Rays[i].texPosX = m_Player.Position.y - wallDistance * rayDirection.y;
            m_Rays[i].brightness = 0.75f;
        }
        else {
            wallDistance = sideDistance.y - deltaDistance.y;
            m_Rays[i].texPosX = m_Player.Position.x + wallDistance * rayDirection.x;
            m_Rays[i].brightness = 1.0f;
        }

        m_Rays[i].rayScaleY = 1.0f / wallDistance;
        m_Rays[i].rayPosX = cameraX + m_RayWidth;
        m_Rays[i].Atlasindex = s_MapData.map[mapY * s_MapData.width + mapX];
        
        m_Lines[i].Scale = rayDirection * wallDistance * s_MapData.mapScale;
    }

    //Floor and ceiling "casting"
    for (uint32_t i = 0; i < m_RayCount; i ++)
    {
        glm::vec3 rayDirection = m_Camera->direction - m_Camera->plane;
        float scale = abs(m_RayCount / (2 * (float)i - m_RayCount)); // = 1.0f / abs(2 * i / m_RayCount - 1)

        m_Sprites[i].TexScale.x = scale;
        m_Sprites[i].TexPosition.x = scale * 0.5f * rayDirection.x + m_Player.Position.x;
        m_Sprites[i].TexPosition.y = scale * 0.5f * rayDirection.y - m_Player.Position.y;
        m_Sprites[i].TexRotation = m_Player.Rotation - 90.0f;
    }
}

void RaycasterScene::ProcessInput(Core::Timestep deltaTime) {
    float velocity = 2.0f * deltaTime;
    float rotationSpeed = 180.0f * deltaTime;

    glm::vec3 front;
    front.x = cos(glm::radians(m_Player.Rotation));
    front.y = -sin(glm::radians(m_Player.Rotation)); //player y is flipped (array index)
    front.z = 0.0f;

    glm::vec3 oldPosition = m_Player.Position;
    if (Core::Input::IsKeyPressed(RC_KEY_W)) {
        m_Player.Position += velocity * front;
    }

    if (Core::Input::IsKeyPressed(RC_KEY_S)) {
        m_Player.Position -= velocity * front;
    }

    if (Core::Input::IsKeyPressed(RC_KEY_A)) {
        m_Player.Rotation += rotationSpeed;
    }

    if (Core::Input::IsKeyPressed(RC_KEY_D)) {
        m_Player.Rotation -= rotationSpeed;
    }

    if (s_MapData.map[(int)oldPosition.y * s_MapData.width + (int)m_Player.Position.x] != 0) {
        m_Player.Position.x = oldPosition.x;
    }
    if (s_MapData.map[(int)m_Player.Position.y * s_MapData.width + (int)m_Player.Position.x] != 0) {
        m_Player.Position.y = oldPosition.y;
    }

    m_Camera->UpdateCamera(m_Player.Position, m_Player.Rotation);
}
