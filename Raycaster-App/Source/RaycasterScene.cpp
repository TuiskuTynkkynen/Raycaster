#include "RaycasterScene.h"

#include <glm/gtx/matrix_transform_2d.hpp>

#include <algorithm>
#include <iostream>

RaycasterScene::MapData RaycasterScene::s_MapData;

void RaycasterScene::Init(){
    m_Rays.resize(2 * m_RayCount); //should be initialized to default values
    m_Lines.resize(m_RayCount); //should be initialized to default vec3s
    m_SpriteObjects.resize(6);

    uint32_t Index = 7; 
    for (uint32_t i = 0; i < m_RayCount; i++) {
        float cameraY = 2 * i / float(m_RayCount) - 1;
        m_Rays[m_RayCount + i].Position.y = cameraY;
        m_Rays[m_RayCount + i].Atlasindex = Index;
        if (i == m_RayCount * 0.5f) { Index = 6; }
    }

    m_Player.Position = glm::vec3((float)s_MapData.width / 2, (float)s_MapData.height / 2, 0.5f);
    m_Player.Scale = s_MapData.mapScale * 0.4f;
    m_Player.Rotation = 90.0f;

    m_Camera = std::make_unique<Core::RaycasterCamera>(m_Player.Position, m_Player.Rotation, s_MapData.mapScalingFactor, s_MapData.width, s_MapData.height);
    
    Core::Tile tile;
    tile.Scale = glm::vec3(0.95f * s_MapData.mapScale.x, 0.95f * s_MapData.mapScale.y, 1.0f);
    tile.Posistion.z = 0.0f;

    float centreY = (float)(s_MapData.height - 1) / 2, centreX = (float)(s_MapData.width - 1) / 2;
    for (uint32_t i = 0; i < s_MapData.size; i++) {
        uint32_t mapX = i % s_MapData.width;
        uint32_t mapY = i / s_MapData.width;

        tile.Posistion.x = (mapX - centreX) * s_MapData.mapScale.x;
        tile.Posistion.y = (centreY - mapY) * s_MapData.mapScale.y;
        
        float brightness = (s_MapData.map[mapY * s_MapData.width + mapX] > 0) ? 1.0f : 0.5f;

        tile.Colour = glm::vec3(brightness);
        m_Tiles.push_back(tile);
    }

    m_Lights.push_back(glm::vec3(2.5f, 3.0f, 0.8f));
    m_Lights.push_back(glm::vec3(8.5f, 6.5f, 0.8f));

    Enemy e;
    e.Position = glm::vec3(8.5f, 6.5f, 0.4f);
    e.Scale = glm::vec3(0.8f);
    e.AtlasIndex = 11;
    e.Speed = 1.0f;
    m_Enemies.push_back(e);

    e.Position = glm::vec3(2.5f, 3.0f, 0.4f);
    m_Enemies.push_back(e);

    for (uint32_t i = 0; i < s_MapData.size; i++) {
        m_EnemyMap[i] = s_MapData.map[i];
    }

    tile.Colour = glm::vec3(1.0f);
    tile.IsTriangle = true;

    int32_t directions[] = {
        1, -1, s_MapData.width, -s_MapData.width, 0 //R, L, D, U 
    };

    for (uint32_t startIndex = 0; startIndex < s_MapData.size; startIndex++) {
        if (s_MapData.map[startIndex] >= 0) {
            continue;
        }

        uint32_t dir1 = 5;
        uint32_t dir2 = 5;
        for (uint32_t i = 0; i < 4; i++) {
            uint32_t cur = startIndex + directions[i];
            if (cur >= s_MapData.size || s_MapData.map[cur] == 0) {
                continue;
            }

            if (dir1 == 5) {
                dir1 = i;
            } else {
                dir2 = i;
            }
        }

        glm::vec2 point1(startIndex % s_MapData.width, startIndex / s_MapData.width);

        tile.Posistion.x = (point1.x - centreX) * s_MapData.mapScale.x;
        tile.Posistion.y = (centreY - point1.y) * s_MapData.mapScale.y;
        tile.Rotation = dir1 == 1 ? 180.0f : 0.0f;
        tile.Rotation -= dir2 - dir1 == 2 ? 90.0f : 0.0f;
        m_Tiles.push_back(tile);
        
        glm::vec2 point2 = point1;

        if (dir2 - dir1 == 2) {
            point1.x++;
            point2.y++;
        }
        else {
            point2.x++;
            point2.y++;
        }

        m_Diagonals.emplace_back(point1.x, point1.y, point2.x, point2.y);
        m_Walls.emplace_back(point1, point2);
    }


    for (uint32_t i = 0; i < 4; i++) {
        std::vector<glm::vec2> points;
        for (uint32_t startIndex = 0; startIndex < s_MapData.size; startIndex++) {
            uint32_t cur = startIndex + directions[i];
            if (s_MapData.map[startIndex] <= 0 || cur >= s_MapData.size) {
                continue;
            }

            if (s_MapData.map[cur] == 0) {
                int f = i == 0 || i == 2 ? i : 4;
                int b = i == 1 || i == 3 ? -1 : 1;

                cur = startIndex + b * directions[(i + 2) % 4] + directions[f];
                glm::vec2 point1((startIndex + directions[f]) % s_MapData.width, (startIndex + directions[f]) / s_MapData.width);
                glm::vec2 point2(cur % s_MapData.width, cur / s_MapData.width);


                auto iter = std::find(points.begin(), points.end(), point1);
                if (iter != points.end()) {
                    *iter = point2;         //if duplicate 
                }
                else {
                    points.push_back(point1);
                    points.push_back(point2);
                }
            }
        }

        for (int j = 0; j < points.size(); j += 2) {
            m_Walls.emplace_back(points[j], points[j +1]);
        }
    }

    tile.Colour = glm::vec3(0.0f, 1.0f, 0.0f);
    tile.Scale = m_Player.Scale;
    tile.IsTriangle = false;
    m_Tiles.push_back(tile);
    m_Tiles.push_back(tile);
}

void RaycasterScene::OnUpdate(Core::Timestep deltaTime) {
    if (!m_Paused) {
        glm::vec3 colour = glm::vec3(0.05f, 0.075f, 0.1f);
        Core::Renderer2D::Clear(colour);

        //Static objects
        m_SpriteObjects[0].Position = glm::vec3(3.0f, 2.5f, 0.25f);
        m_SpriteObjects[0].Scale = glm::vec3(0.5f, 0.5f, 0.5f);
        m_SpriteObjects[0].AtlasIndex = 8;
        m_SpriteObjects[0].FlipTexture = false;
        
        m_SpriteObjects[1].Position = glm::vec3(2.5f, 2.5f, 0.25f);
        m_SpriteObjects[1].Scale = glm::vec3(0.5f, 0.5f, 0.5f);
        m_SpriteObjects[1].AtlasIndex = 8;
        m_SpriteObjects[1].FlipTexture = false;

        m_SpriteObjects[2].Position = glm::vec3(2.5f, 3.0f, 0.85f);
        m_SpriteObjects[2].Scale = glm::vec3(0.5f, 0.5f, 0.5f);
        m_SpriteObjects[2].AtlasIndex = 10;
        m_SpriteObjects[2].FlipTexture = false;

        m_SpriteObjects[3].Position = glm::vec3(8.5f, 6.5f, 0.85f);
        m_SpriteObjects[3].Scale = glm::vec3(0.5f, 0.5f, 0.5f);
        m_SpriteObjects[3].AtlasIndex = 10;
        m_SpriteObjects[3].FlipTexture = false;

        ProcessInput(deltaTime);
        UpdateEnemies(deltaTime);
        CastRays();
        RenderSprites();
    }
}

void RaycasterScene::CastRays() {
    //Calculate and sort distance to each diagonal
    glm::mat3 matrix = glm::rotate(glm::mat3(1.0f), glm::radians(m_Player.Rotation + 90.0f));
    std::vector<glm::i32vec2> diagonalDistances;

    uint32_t diagonalCount = m_Diagonals.size();
    for (uint32_t i = 0; i < diagonalCount; i++) {
        int32_t distance = m_Diagonals[i].x + m_Diagonals[i].y - m_Player.Position.x - m_Player.Position.y;
        diagonalDistances.emplace_back(distance, i);
    }

    std::sort(diagonalDistances.begin(), diagonalDistances.end(), [this](glm::i32vec2& a, glm::i32vec2& b) {
        return a.x > b.x;
    });

    //Wall casting
    for (uint32_t i = 0; i < m_RayCount; i++) {
        float cameraX = 2 * i / float(m_RayCount) - 1;
        glm::vec3 rayDirection = m_Camera->direction + m_Camera->plane * cameraX;
        glm::vec3 deltaDistance = glm::abs((float)1 / rayDirection);

        uint32_t mapX = m_Player.Position.x;
        uint32_t mapY = m_Player.Position.y;

        int32_t stepX = (rayDirection.x > 0) ? 1 : -1;
        int32_t stepY = (rayDirection.y < 0) ? 1 : -1;

        glm::vec3 sideDistance = deltaDistance;
        sideDistance.x *= (rayDirection.x < 0) ? (m_Player.Position.x - mapX) : (mapX + 1.0f - m_Player.Position.x);
        sideDistance.y *= (rayDirection.y > 0) ? (m_Player.Position.y - mapY) : (mapY + 1.0f - m_Player.Position.y);

        bool hit = false;
        uint32_t side = 0;
        glm::vec2 worldPosition;
        while (!hit){
            //if diagonal, needs to be handled first, because player can be inside diagonal
            if (s_MapData.map[mapY * s_MapData.width + mapX] < 0) {
                glm::vec2 point3(m_Player.Position.x, m_Player.Position.y);
                glm::vec2 point4 = point3;
                point3.x += rayDirection.x;
                point3.y -= rayDirection.y;

                for (uint32_t j = 0; j < diagonalCount; j++) {
                    int32_t dist = mapX + mapY - m_Player.Position.x - m_Player.Position.y - diagonalDistances[j].x;
                    if (dist > 2 || dist < -2) {
                        continue;
                    }
                    uint32_t index = diagonalDistances[j].y;

                    glm::vec2 p1(m_Diagonals[index].x, m_Diagonals[index].y);
                    glm::vec2 p2(m_Diagonals[index].z, m_Diagonals[index].w);
                    std::optional<glm::vec2> intersection = Algorithms::LineIntersection(p1, p2, point3, point4, true);
                    if (!intersection || (uint32_t)intersection.value().x != mapX || (uint32_t)intersection.value().y != mapY) {
                        continue;
                    }

                    worldPosition = intersection.value();
                    sideDistance.x = intersection.value().x;
                    sideDistance.y = intersection.value().y;
                    sideDistance.z = 0.0f;
                    sideDistance.y = (matrix * (sideDistance - m_Player.Position)).y; //Calculates perpendicular distance

                    side = 2;
                    hit = true;
                    break;
                }
                if (hit) {
                    break;
                }
            }

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

            if (mapY >= s_MapData.height || mapX >= s_MapData.width) {
                std::cout << "ERROR: INDEX OUT OF BOUNDS" << std::endl;
                break;
            }

            if (s_MapData.map[mapY * s_MapData.width + mapX] > 0) {
                hit = true;
            }
        }

        float wallDistance;
        if (side == 0) {
            wallDistance = sideDistance.x - deltaDistance.x;

            float offset = m_Player.Position.y - wallDistance * rayDirection.y;
            offset -= floor(offset);
            worldPosition.x = mapX - stepX;
            worldPosition.y = mapY + offset;

            m_Rays[i].TexPosition.x = offset;
        }
        else if (side == 1) {
            wallDistance = sideDistance.y - deltaDistance.y;

            float offset = m_Player.Position.x + wallDistance * rayDirection.x;
            offset -= floor(offset);
            worldPosition.x = mapX + offset;
            worldPosition.y = mapY - stepY;

            m_Rays[i].TexPosition.x = offset;
        }
        else {
            wallDistance = sideDistance.y;
            m_Rays[i].TexPosition.x = sideDistance.x;
        }

        m_Rays[i].Scale = 1.0f / wallDistance;
        m_Rays[i].Position.x = cameraX + m_RayWidth;
        m_Rays[i].Atlasindex = abs(s_MapData.map[mapY * s_MapData.width + mapX]);

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
    for (uint32_t i = 0; i < m_RayCount; i++) {
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
    uint32_t count = m_SpriteObjects.size();
    uint32_t rayIndex = 2 * m_RayCount;
    uint32_t space = m_Rays.size();

    glm::mat3 matrix = glm::rotate(glm::mat3(1.0f), glm::radians(m_Player.Rotation + 90.0f));

    for (uint32_t index = 0; index < count; index++) {
        m_SpriteObjects[index].Position -= m_Player.Position;
        m_SpriteObjects[index].Position = matrix * m_SpriteObjects[index].Position;
    }

    std::sort(m_SpriteObjects.begin(), m_SpriteObjects.end(), [this](SpriteObject a, SpriteObject b) {
        return a.Position.y > b.Position.y;
    });

    for (uint32_t index = 0; index < count; index++) {
        glm::vec3 position = m_SpriteObjects[index].Position;
        
        if (position.y < 0) {
            break;
        }

        glm::vec3 scale = m_SpriteObjects[index].Scale;
        uint32_t atlasIndex = m_SpriteObjects[index].AtlasIndex;
        bool flipTexture = m_SpriteObjects[index].FlipTexture;

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
        float texturePosition;

        for (int32_t i = startX; i < endX; i++) {
            if (i >= m_RayCount || m_ZBuffer[i] < distance) {
                continue;
            }

            if (rayIndex >= space) {
                Core::Ray r;
                m_Rays.push_back(r);
            }

            position.x = (i + 0.5f) * rScale - 1.0f;
            texturePosition = std::max((i - startX) / width, 0.0f);

            m_Rays[rayIndex].Position = position;
            m_Rays[rayIndex].Scale = scale.y;
            m_Rays[rayIndex].TexPosition.x = (flipTexture) ? 1.0f - texturePosition : texturePosition;
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

    glm::vec2 col(m_Player.Position.x, m_Player.Position.y);
    col = Algorithms::LineCollisions(col, m_Walls, 0.4f);

    float length = glm::length(col);
    if (length > velocity) {
        col *= 1.0f / length * velocity;
    }

    m_Player.Position.x += col.x;
    m_Player.Position.y += col.y;

    m_Camera->UpdateCamera(m_Player.Position, m_Player.Rotation);
}

void RaycasterScene::UpdateEnemies(Core::Timestep deltaTime) {
    uint32_t count = m_Enemies.size();
    uint32_t tileIndex = m_Tiles.size() - 1;
    
    for (uint32_t i = 0; i < count; i++) {
        Enemy& enemy = m_Enemies[i];
        uint32_t atlasOffset = 0;

        uint32_t mapIndex = (uint32_t)enemy.Position.y * s_MapData.width + (uint32_t)enemy.Position.x;
        m_EnemyMap[mapIndex] = s_MapData.map[mapIndex];
        
        glm::vec3 distance = enemy.Position - m_Player.Position;
        if (glm::length(distance) < 1.1f) {
            //"Attack"
            enemy.Tick += deltaTime;
            atlasOffset = 1;
        } else {
            //Pathfinding
            enemy.Tick += deltaTime * 2.0f;
            bool lineOfSight = true;

            glm::vec2 playerPos;
            playerPos.x = m_Player.Position.x;
            playerPos.y = m_Player.Position.y;

            glm::vec2 movementVector = playerPos;

            glm::vec2 enemyPos;
            static const glm::i32vec2 directions[] = {
                glm::i32vec2(1,0),
                glm::i32vec2(-1,0),
                glm::i32vec2(0,1),
                glm::i32vec2(0,-1),
                glm::i32vec2(1,1),
                glm::i32vec2(-1,1),
                glm::i32vec2(1,-1),
                glm::i32vec2(-1,-1),
            };
            for (uint32_t j = 0; j < 8 && lineOfSight; j++) {
                enemyPos.x = enemy.Position.x + 0.5f * enemy.Scale.x * directions[j].x;
                enemyPos.y = enemy.Position.y + 0.5f * enemy.Scale.y * directions[j].y;

                lineOfSight &= Algorithms::LineOfSight(enemyPos, playerPos, m_EnemyMap, s_MapData.width, s_MapData.height);
            }

            if (!lineOfSight) {
                enemyPos.x = enemy.Position.x;
                enemyPos.y = enemy.Position.y;

                movementVector = Algorithms::AStar(enemyPos, playerPos, m_EnemyMap, s_MapData.width, s_MapData.height);
                movementVector += 0.5f; //Pathfind to tile centre
            }

            movementVector.x -= enemy.Position.x;
            movementVector.y -= enemy.Position.y;
            movementVector = glm::normalize(movementVector);
            movementVector *= deltaTime * enemy.Speed;

            enemy.Position.x += movementVector.x;
            enemy.Position.y += movementVector.y;
        }

        m_EnemyMap[(uint32_t)enemy.Position.y * s_MapData.width + (uint32_t)enemy.Position.x] = true;
        m_SpriteObjects[4 + i].Position = enemy.Position;
        m_SpriteObjects[4 + i].Scale = enemy.Scale;
        m_SpriteObjects[4 + i].AtlasIndex = enemy.AtlasIndex + atlasOffset;
        m_SpriteObjects[4 + i].FlipTexture = (uint32_t)enemy.Tick % 2 == 0;

        //Update on 2D-layer
        uint32_t centreY = s_MapData.height * 0.5f, centreX = s_MapData.width * 0.5f;
        m_Tiles[tileIndex - i].Posistion.x = (enemy.Position.x - centreX) * s_MapData.mapScale.x;
        m_Tiles[tileIndex - i].Posistion.y = (centreY - enemy.Position.y) * s_MapData.mapScale.y;
    }
}