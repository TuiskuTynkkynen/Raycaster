#include "RaycasterScene.h"

#include <glm/gtx/matrix_transform_2d.hpp>

#include <algorithm>
#include <iostream>

RaycasterScene::MapData RaycasterScene::s_MapData;

void RaycasterScene::Init(){
    m_Rays.resize(2 * m_RayCount); //should be initialized to default values
    m_Lines.resize(m_RayCount); //should be initialized to default vec3s
    
    uint32_t Index = 7; 
    for (uint32_t i = 0; i < m_RayCount; i++) {
        float cameraY = 2 * i / float(m_RayCount) - 1;
        m_Rays[m_RayCount + i].Position.y = cameraY;
        m_Rays[m_RayCount + i].Atlasindex = Index;
        if (i == m_RayCount * 0.5f) { Index = 6; }
    }

    m_Player.Position = glm::vec3((float)s_MapData.width / 2, (float)s_MapData.heigth / 2, 0.5f);
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

    m_Lights.push_back(glm::vec3(2.5f, 3.0f, 0.8f));
    m_Lights.push_back(glm::vec3(8.0f, 6.0f, 0.8f));
}

void RaycasterScene::OnUpdate(Core::Timestep deltaTime) {
    if (!m_Paused) {
        glm::vec3 colour = glm::vec3(0.05f, 0.075f, 0.1f);
        Core::Renderer2D::Clear(colour);
        ProcessInput(deltaTime);
        CastRays();
        RenderSprites();
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

        glm::vec2 worldPosition(mapX, mapY);
        float wallDistance;
        if (side == 0) {
            wallDistance = sideDistance.x - deltaDistance.x;
            
            float offset = m_Player.Position.y - wallDistance * rayDirection.y;
            offset -= floor(offset);
            worldPosition.y += offset;
            
            m_Rays[i].TexPosition.x = m_Player.Position.y - wallDistance * rayDirection.y;
        }
        else {
            wallDistance = sideDistance.y - deltaDistance.y;
            
            float offset = m_Player.Position.x + wallDistance * rayDirection.x;
            offset -= floor(offset);
            worldPosition.x += offset;
            
            m_Rays[i].TexPosition.x = m_Player.Position.x + wallDistance * rayDirection.x;
        }

        m_Rays[i].Scale = 1.0f / wallDistance;
        m_Rays[i].Position.x = cameraX + m_RayWidth;
        m_Rays[i].Atlasindex = s_MapData.map[mapY * s_MapData.width + mapX];

        float brightness = 0.0f;
        for (glm::vec2 lightPos : m_Lights) {
            float distance = glm::length(worldPosition - lightPos);
            brightness += 1.0f / (0.95f + 0.1f * distance + 0.03f * (distance * distance));
        }
        m_Rays[i].Brightness = brightness;

        m_ZBuffer[i] = wallDistance;
        m_Lines[i].Scale = rayDirection * wallDistance * s_MapData.mapScale;
    }

    //Floor and ceiling "casting"
    for (uint32_t i = 0; i < m_RayCount; i ++)
    {
        glm::vec3 rayDirection = m_Camera->direction - m_Camera->plane;
        float scale = abs(m_RayCount / (2 * (float)i - m_RayCount)); // = 1.0f / abs(2 * i / m_RayCount - 1)
        
        m_Rays[m_RayCount + i].Scale = scale;
        m_Rays[m_RayCount + i].TexPosition.x = scale * 0.5f * rayDirection.x + m_Player.Position.x;
        m_Rays[m_RayCount + i].TexPosition.y = scale * 0.5f * rayDirection.y - m_Player.Position.y;

        glm::vec2 worldPosition(scale * 0.5f * m_Camera->direction.x + m_Player.Position.x, scale * 0.5f * -m_Camera->direction.y + m_Player.Position.y);

        float brightness = 0.0f;
        for (glm::vec2 lightPos : m_Lights) {
            float distance = glm::length(worldPosition - lightPos);
            brightness += std::min(1.0f / (0.95f + 0.1f * distance + 0.03f * (distance * distance)), 1.0f);
        }
        m_Rays[m_RayCount + i].Brightness = brightness;
        
        m_Rays[m_RayCount + i].TexRotation = m_Player.Rotation - 90.0f;
    }
}

void RaycasterScene::RenderSprites() {
    //TODO use actual objects instead of mocks 
    std::vector<glm::vec3> positions;
    positions.push_back(glm::vec3(3.0f, 2.5f, 0.2f));
    positions.push_back(glm::vec3(2.5f, 2.5f, 0.2f));
    
    uint32_t count = positions.capacity();
    uint32_t rayIndex = 2 * m_RayCount;
    uint32_t space = m_Rays.size();

    glm::mat3 matrix = glm::rotate(glm::mat3(1.0f), glm::radians(m_Player.Rotation + 90.0f));

    for (uint32_t index = 0; index < count; index++) {
        positions[index] -= m_Player.Position;
        positions[index] = matrix * positions[index];
    }

    std::sort(positions.begin(), positions.end(), [this](glm::vec3 a, glm::vec3 b) {
        return a.y > b.y;
    });

    for (uint32_t index = 0; index < count; index++) {
        glm::vec3 position = positions[index];
        
        if (position.y < 0) {
            break;
        }

        glm::vec3 scale(0.5f, 0.5f, 0.5f);
        glm::vec3 colour(1.0f, 1.0f, 1.0f);
        uint32_t atlasIndex = 8;

        float brightness = 0.0f;
        for (glm::vec3 lightPos : m_Lights) {
            lightPos -= m_Player.Position;
            lightPos = matrix * lightPos;  //TODO test copying light vec instead of calculating multiple times
            float distance = glm::length(position - lightPos);
            brightness += std::min(1.0f / (0.95f + 0.1f * distance + 0.03f * (distance * distance)), 1.0f);
        }

        float distance = position.y;
        position.x *= -1.0f / distance;
        scale /= distance;
        position.y = position.z / distance;

        float rScale = 2.0f / m_RayCount;
        float width = scale.x * m_RayCount * 0.5f;
        float startX = 0.5f * (m_RayCount - width + position.x * m_RayCount);
        float endX = startX + width;
        scale.x = rScale;
        
        for (int32_t i = startX; i < endX; i++) {
            if (i >= m_RayCount || m_ZBuffer[i] < distance) {
                continue;
            }

            if (rayIndex >= space) {
                Core::Ray r;
                m_Rays.push_back(r);
            }

            position.x = (i + 0.5f) * rScale - 1.0f;
            
            m_Rays[rayIndex].Position = position;
            m_Rays[rayIndex].Scale = scale.y;
            m_Rays[rayIndex].TexPosition.x = std::max((i - startX) / width, 0.0f);
            m_Rays[rayIndex].Atlasindex = atlasIndex;
            m_Rays[rayIndex].Brightness =  brightness;

            rayIndex++;
        }
    }

    m_Rays.resize(rayIndex);
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
