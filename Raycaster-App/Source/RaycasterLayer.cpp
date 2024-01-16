#include "RaycasterLayer.h"

#include <iostream>

struct SceneData
{
    static const uint32_t heigth = 6, width = 5;
    static const uint32_t size = heigth * width;
    static const uint32_t rayCount = 200;

    const uint32_t map[size]{
        1, 1, 1, 1, 1,
        1, 1, 0, 1, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 1, 0, 1, 1,
        1, 1, 1, 1, 1,
    };

    float centre;
    glm::vec3 tilePositions[size];
    glm::vec3 mapScale, tileScale;

    glm::vec3 playerPosition, playerScale;
    float playerRotation;
};

static SceneData s_Data;

void RaycasterLayer::OnAttach() {
    const float centreY = (float)(s_Data.heigth - 1) / 2, centreX = (float)(s_Data.width - 1) / 2;
    const float mapScalingFactor = 1.4f;

    s_Data.centre = sqrt((float)s_Data.size) / mapScalingFactor;
    
    for (uint32_t i = 0; i < s_Data.size; i++) {
        uint32_t mapX = i % s_Data.width;
        uint32_t mapY = i / s_Data.width;

        float worldX = (mapX - centreX) / (s_Data.centre);
        float worldY = (centreY - mapY) / (s_Data.centre);
        float worldZ = 0.0f;
        s_Data.tilePositions[i] = glm::vec3(worldX, worldY, worldZ);
    }

    s_Data.mapScale = glm::vec3(1 / s_Data.centre, 1 / s_Data.centre, 1.0f);
    s_Data.tileScale = glm::vec3(0.95f / s_Data.centre, 0.95f / s_Data.centre, 1.0f);
    
    s_Data.playerPosition = glm::vec3((float)s_Data.width / 2, (float)s_Data.heigth / 2, 1.0f);
    s_Data.playerScale = s_Data.mapScale * 0.4f;
    s_Data.playerRotation = 90.0f;

    m_Camera = new Core::RaycasterCamera(s_Data.playerPosition, s_Data.playerRotation, s_Data.centre, s_Data.width, s_Data.heigth);
}

void RaycasterLayer::OnUpdate() {
    glm::vec3 AxisZ(0.0f, 0.0f, 1.0f);
    glm::vec3 zero(0.0f);
    glm::mat4 identity(1.0f);

	float lineWidth = 1.0f / s_Data.rayCount;
	glm::vec3 line = glm::vec3(1.0f);
	line.x = lineWidth * 2;
	glm::vec3 linePos = glm::vec3(0.0f);
	glm::vec2 texPos = glm::vec2(0.0f);
	glm::vec2 texScale = glm::vec2(0.0f, 1.0f);

    glm::vec3 rays[s_Data.rayCount];

    glm::vec3 colour = glm::vec3(0.05f, 0.075f, 0.1f);
    Core::Renderer2D::Clear(colour);

    Core::Renderer2D::SetViewPort(0, 0, 600, 600);

    for (uint32_t i = 0; i < s_Data.rayCount; i++)
    {
        float cameraX = 2 * i / float(s_Data.rayCount) - 1;
        glm::vec3 rayDirection = m_Camera->direction + m_Camera->plane * cameraX;
        glm::vec3 deltaDistance = glm::abs((float)1 / rayDirection);

        uint32_t mapX = (int)s_Data.playerPosition.x;
        uint32_t mapY = (int)s_Data.playerPosition.y;

        int32_t stepX = (rayDirection.x > 0) ? 1 : -1;
        int32_t stepY = (rayDirection.y < 0) ? 1 : -1;

        glm::vec3 sideDistance = deltaDistance;
        sideDistance.x *= (rayDirection.x < 0) ? (s_Data.playerPosition.x - mapX) : (mapX + 1.0f - s_Data.playerPosition.x);
        sideDistance.y *= (rayDirection.y > 0) ? (s_Data.playerPosition.y - mapY) : (mapY + 1.0f - s_Data.playerPosition.y);

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

            if (mapY >= s_Data.heigth || mapX >= s_Data.width) {
                std::cout << "ERROR: INDEX OUT OF BOUNDS" << std::endl;
                break;
            }
            if (s_Data.map[mapY * s_Data.width + mapX] > 0) {
                hit = 1;
            }
        }

        float wallDistance;
        if (side == 0) {
            wallDistance = sideDistance.x - deltaDistance.x;
            texPos.x = s_Data.playerPosition.y - wallDistance * rayDirection.y;
            colour = glm::vec3(0.75f);
        }
        else {
            wallDistance = sideDistance.y - deltaDistance.y;
            texPos.x = s_Data.playerPosition.x + wallDistance * rayDirection.x;
            colour = glm::vec3(1.0f);
        }

        line.y = 1.0f / wallDistance;
        linePos.x = cameraX + lineWidth;
        Core::Renderer2D::DrawTextureQuad(linePos, line, colour, texPos, texScale);

        glm::vec3 ray = rayDirection * wallDistance * s_Data.mapScale;
        ray.z = 0;
        rays[i] = ray;
    }

    Core::Renderer2D::SetViewPort(600, 0, 600, 600);
    Core::Renderer2D::BeginScene(*m_Camera);
	for (int i = 0; i < s_Data.size; i++) {
		colour = (s_Data.map[i] != 0) ? glm::vec3(0.8f) : glm::vec3(0.2f);
        Core::Renderer2D::DrawFlatQuad(s_Data.tilePositions[i], s_Data.tileScale, colour);
	}

    Core::Renderer2D::BeginScene(identity);

	colour = glm::vec3(1.0f, 0.0f, 0.0f);
    Core::Renderer2D::DrawRotatedFlatQuad(zero, s_Data.playerRotation, AxisZ, s_Data.playerScale, colour);

	colour = glm::vec3(0.0f, 0.0f, 1.0f);
    for (int i = 0; i < s_Data.rayCount; i++) {
        Core::Renderer2D::DrawLine(zero, rays[i], colour);
    }
}
