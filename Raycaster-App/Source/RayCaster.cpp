#include "RayCaster.h"

#include <iostream>

Raycaster::Raycaster() {
}

Raycaster::~Raycaster() {
}

void Raycaster::Run() {
	Core::Renderer2D renderer;

	const uint32_t map[]{
		1, 1, 1, 1, 1,
		1, 1, 0, 1, 1,
		1, 0, 0, 0, 1,
		1, 0, 0, 0, 1,
		1, 1, 0, 1, 1,
		1, 1, 1, 1, 1,
	};

	const uint32_t heigth = 6, width = 5;
	const float centreY = (float)(heigth - 1) / 2, centreX = (float)(width - 1) / 2;
	const uint32_t size = heigth * width;
	const float mapScalingFactor = 1.4f;
	const float centre = sqrt((float)size) / mapScalingFactor;

	glm::vec3 tilePositions[size];
	for (uint32_t i = 0; i < size; i++) {
		uint32_t mapX = i % width;
		uint32_t mapY = i / width;

		float worldX = (mapX - centreX) / (centre);
		float worldY = (centreY - mapY) / (centre);
		float worldZ = 0.0f;
		tilePositions[i] = glm::vec3(worldX, worldY, worldZ);
	}

	glm::vec3 mapScale = glm::vec3(1 / centre, 1 / centre, 1.0f);
	glm::vec3 tileScale = glm::vec3(0.95f / centre, 0.95f / centre, 1.0f);
	glm::vec3 colour;

	glm::vec3 playerPosition = glm::vec3((float)width / 2, (float)heigth / 2, 1.0f);
	glm::vec3 playerScale = mapScale * 0.4f;
	float playerRotation = 90.0f;

	Core::RaycasterCamera camera(playerPosition, playerRotation, centre, width, heigth);

	const uint32_t rayCount = 200;

	float lineWidth = 1.0f / rayCount;
	glm::vec3 line = glm::vec3(1.0f);
	line.x = lineWidth * 2;
	glm::vec3 linePos = glm::vec3(0.0f);
	glm::vec2 texPos = glm::vec2(0.0f);
	glm::vec2 texScale = glm::vec2(0.0f, 1.0f);

	glm::vec3 rays[rayCount];

	glm::vec3 AxisZ = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 zero(0.0f);
    glm::mat4 identity(1.0f);

	while (m_Running)
	{
		colour = glm::vec3(0.05f, 0.075f, 0.1f);
		renderer.Clear(colour);

		renderer.SetViewPort(0, 0, 600, 600);

        for (uint32_t i = 0; i < rayCount; i++)
        {
            float cameraX = 2 * i / float(rayCount) - 1;
            glm::vec3 rayDirection = camera.direction + camera.plane * cameraX;
            glm::vec3 deltaDistance = glm::abs((float)1 / rayDirection);

            uint32_t mapX = (int)playerPosition.x;
            uint32_t mapY = (int)playerPosition.y;

            int32_t stepX = (rayDirection.x > 0) ? 1 : -1;
            int32_t stepY = (rayDirection.y < 0) ? 1 : -1;

            glm::vec3 sideDistance = deltaDistance;
            sideDistance.x *= (rayDirection.x < 0) ? (playerPosition.x - mapX) : (mapX + 1.0f - playerPosition.x);
            sideDistance.y *= (rayDirection.y > 0) ? (playerPosition.y - mapY) : (mapY + 1.0f - playerPosition.y);

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

                if (mapY >= heigth || mapX >= width) {
                    std::cout << "ERROR: INDEX OUT OF BOUNDS" << std::endl;
                    break;
                }
                if (map[mapY * width + mapX] > 0) {
                    hit = 1;
                }
            }

            float wallDistance;
            if (side == 0) {
                wallDistance = sideDistance.x - deltaDistance.x;
                texPos.x = playerPosition.y - wallDistance * rayDirection.y;
                colour = glm::vec3(0.75f);
            }
            else {
                wallDistance = sideDistance.y - deltaDistance.y;
                texPos.x = playerPosition.x + wallDistance * rayDirection.x;
                colour = glm::vec3(1.0f);
            }

            line.y = 1.0f / wallDistance;
            linePos.x = cameraX + lineWidth;
            renderer.DrawTextureQuad(linePos, line, colour, texPos, texScale);

            glm::vec3 ray = rayDirection * wallDistance * mapScale;
            ray.z = 0;
            rays[i] = ray;
        }

        renderer.SetViewPort(600, 0, 600, 600);
		renderer.BeginScene(camera);
		for (int i = 0; i < size; i++) {
			colour = (map[i] != 0) ? glm::vec3(0.8f) : glm::vec3(0.2f);
			renderer.DrawFlatQuad(tilePositions[i], tileScale, colour);
		}

        renderer.BeginScene(identity);
		colour = glm::vec3(1.0f, 0.0f, 0.0f);
		renderer.DrawRotatedFlatQuad(zero, playerRotation, AxisZ, playerScale, colour);

		colour = glm::vec3(0.0f, 0.0f, 1.0f);
        for (int i = 0; i < rayCount; i++) {
            renderer.DrawLine(zero, rays[i], colour);
        }

		m_Window->OnUpdate();
	}
}