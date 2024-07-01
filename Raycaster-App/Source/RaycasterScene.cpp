#include "RaycasterScene.h"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>

#include <algorithm>
#include <ranges>
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
    m_Camera3D = std::make_unique<Core::FlyCamera>(glm::vec3(m_Player.Position.x, 0.5f, m_Player.Position.y), glm::vec3(0.0f, 1.0f, 0.0f), -m_Player.Rotation, 0.0f);
    
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

    m_Lights.push_back(glm::vec3(2.5f, 3.0f, 0.75f));
    m_Lights.push_back(glm::vec3(8.5f, 6.5f, 0.75f));
    
    SpriteObject staticObject;
    staticObject.Position = glm::vec3(3.0f, 2.5f, 0.25f);
    staticObject.Scale = glm::vec3(0.5f, 0.5f, 0.5f);
    staticObject.AtlasIndex = 8;
    staticObject.FlipTexture = false;
    
    m_StaticObjects.push_back(staticObject);
    staticObject.Position.x = 2.5f;
    m_StaticObjects.push_back(staticObject);

    staticObject.AtlasIndex = 10;
    for (const glm::vec3& light : m_Lights) {
        staticObject.Position = light;
        m_StaticObjects.push_back(staticObject);
    }

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

    InitWalls();
    InitModels();

    tile.Colour = glm::vec3(0.0f, 1.0f, 0.0f);
    tile.Scale = m_Player.Scale;
    m_Tiles.push_back(tile);
    m_Tiles.push_back(tile);

    Core::RenderAPI::SetClearColour(glm::vec3(0.05f, 0.075f, 0.1f));
}

void RaycasterScene::OnUpdate(Core::Timestep deltaTime) {
    if (!m_Paused) {
        Core::RenderAPI::Clear();

        //Static objects
        for (uint32_t i = 0; i < m_StaticObjects.size(); i++) {
            m_SpriteObjects[i] = m_StaticObjects[i];
        }

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
        glm::vec3 rayDirection = m_Camera->GetDirection() + m_Camera->GetPlane() * cameraX;
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
        glm::vec3 rayDirection = m_Camera->GetDirection() - m_Camera->GetPlane();
        float scale = abs(m_RayCount / (2 * (float)i - m_RayCount)); // = 1.0f / abs(2 * i / m_RayCount - 1)

        m_Rays[m_RayCount + i].Scale = scale;
        m_Rays[m_RayCount + i].TexPosition.x = scale * 0.5f * rayDirection.x + m_Player.Position.x;
        m_Rays[m_RayCount + i].TexPosition.y = scale * 0.5f * rayDirection.y - m_Player.Position.y;

        glm::vec2 worldPosition(scale * 0.5f * m_Camera->GetDirection().x + m_Player.Position.x, scale * 0.5f * -m_Camera->GetDirection().y + m_Player.Position.y);

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
        //3D
        glm::vec3 position3D(m_SpriteObjects[index].Position.x, m_SpriteObjects[index].Position.z, m_SpriteObjects[index].Position.y);
        m_Models[index + 1].Transform = glm::translate(glm::mat4(1.0f), position3D);
        m_Models[index + 1].Transform = glm::rotate(m_Models[index + 1].Transform, glm::radians(m_Player.Rotation - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        
        //Transform for 2D
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

    glm::vec3 front(0.0f);
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
    m_Camera3D->UpdateCamera(glm::vec3(m_Player.Position.x, 0.5f, m_Player.Position.y), -m_Player.Rotation);
}

void RaycasterScene::UpdateEnemies(Core::Timestep deltaTime) {
    uint32_t count = m_Enemies.size();
    uint32_t tileIndex = m_Tiles.size() - 1;
    uint32_t modelIndex = m_Models.size() - count;
    
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

            glm::vec2 playerPos(0.0f);
            playerPos.x = m_Player.Position.x;
            playerPos.y = m_Player.Position.y;

            glm::vec2 movementVector = playerPos;

            glm::vec2 enemyPos(0.0f);
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
        bool flip = (uint32_t)enemy.Tick % 2 == 0;
        m_SpriteObjects[4 + i].FlipTexture = flip;

        //Update on 2D-layer
        uint32_t centreY = s_MapData.height * 0.5f, centreX = s_MapData.width * 0.5f;
        m_Tiles[tileIndex - i].Posistion.x = (enemy.Position.x - centreX) * s_MapData.mapScale.x;
        m_Tiles[tileIndex - i].Posistion.y = (centreY - enemy.Position.y) * s_MapData.mapScale.y;

        //Update on 3D-layer
        uint32_t atlasWidth = 11;
        glm::vec2 index = glm::vec2((enemy.AtlasIndex + atlasOffset) % atlasWidth, (enemy.AtlasIndex + atlasOffset) / atlasWidth);
        m_Models[modelIndex + i].Materials.front()->Parameters.back().Value = glm::vec2(flip ? 0.0f : 1.0f, 0.0f);
        m_Models[modelIndex + i].Materials.front()->Parameters.front().Value = index;
    }
}

void RaycasterScene::InitWalls() {
    const int32_t directions[] = {
        1, -1, s_MapData.width, -s_MapData.width, 0 //R, L, D, U 
    };
    const float centreY = (s_MapData.height - 1.0f) / 2, centreX = (s_MapData.width - 1.0f) / 2;

    Core::Tile tile;
    tile.Scale = glm::vec3(0.95f * s_MapData.mapScale.x, 0.95f * s_MapData.mapScale.y, 1.0f);
    tile.Colour = glm::vec3(1.0f);
    tile.IsTriangle = true;
    
    glm::vec2 point1(0.0f), point2(0.0f);

    for (uint32_t startIndex = 0; startIndex < s_MapData.size; startIndex++) {
        if (s_MapData.map[startIndex] >= 0) {
            continue;
        }

        point1.x = point2.x = startIndex % s_MapData.width;
        point1.y = point2.y = startIndex / s_MapData.width;
        
        uint32_t dir1 = 5;
        uint32_t dir2 = 5;
        for (uint32_t i = 0; i < 4; i++) {
            uint32_t test = startIndex + directions[i];
         
            if (test >= s_MapData.size || s_MapData.map[test] == 0) {
                continue;
            }

            if (dir1 == 5) {
                dir1 = i;
            } else {
                dir2 = i;
            }
        }
        
        tile.Posistion.x = (point1.x - centreX) * s_MapData.mapScale.x;
        tile.Posistion.y = (centreY - point1.y) * s_MapData.mapScale.y;
        tile.Rotation = dir1 == 1 || dir1 + dir2 > 5 ? 180.0f : 0.0f;
        tile.Rotation -= dir2 - dir1 == 2 ? 90.0f : 0.0f;
        m_Tiles.push_back(tile);

        if (dir2 - dir1 == 2) {
            point1.x++;
            point2.y++;
        } else {
            point2++;
        }

        m_Diagonals.emplace_back(point1.x, point1.y, point2.x, point2.y);
        m_Walls.emplace_back(point1, point2);
    }

    std::vector<glm::vec2> points;
    for (uint32_t i = 0; i < 4; i++) {
        for (uint32_t startIndex = 0; startIndex < s_MapData.size; startIndex++) {
            uint32_t current = startIndex + directions[i];
            if (s_MapData.map[startIndex] <= 0 || current >= s_MapData.size || s_MapData.map[current] != 0) {
                continue;
            }
            
            int32_t offset = i % 2 == 0 ? directions[i] : 0;
            current = startIndex + offset;
    
            point1.x = current % s_MapData.width;
            point1.y = current / s_MapData.width;

            current += abs(directions[(i + 2) % 4]); // rotate 2nd point 90 deg ahead

            point2.x = current % s_MapData.width;
            point2.y = current / s_MapData.width;

            auto iterator = std::find(points.begin(), points.end(), point1);
            if (iterator != points.end()) {
                *iterator = point2;         //if 1st point already exist, line end point can be updated
            } else {
                points.push_back(point1);
                points.push_back(point2);
            }
        }

        for (int j = 0; j < points.size(); j += 2) {
            m_Walls.emplace_back(points[j], points[j + 1]);
        }
        points.clear();
    }
}

void RaycasterScene::InitModels() {
    std::vector<std::pair<glm::vec4, uint32_t>> walls;
   
    {
        //Add horizontal and vertical walls to walls vector
        const int32_t directions[] = {
            1, -1, s_MapData.width, -s_MapData.width, 0 //R, L, D, U 
        };

        glm::vec3 point1(0.0f), point2(0.0f);
        std::vector<glm::vec3> points;
        for (uint32_t i = 0; i < 4; i++) {
            for (uint32_t startIndex = 0; startIndex < s_MapData.size; startIndex++) {
                uint32_t current = startIndex + directions[i];
                if (s_MapData.map[startIndex] <= 0 || current >= s_MapData.size || s_MapData.map[current] != 0) {
                    continue;
                }

                int32_t offset = i % 2 == 0 ? directions[i] : 0;
                current = startIndex + offset;

                point1.z = point2.z = s_MapData.map[startIndex];
                point1.x = current % s_MapData.width;
                point1.y = current / s_MapData.width;

                current += abs(directions[(i + 2) % 4]); // rotate 2nd point 90 deg ahead

                point2.x = current % s_MapData.width;
                point2.y = current / s_MapData.width;

                auto iterator = std::find(points.begin(), points.end(), point1);
                if (iterator != points.end()) {
                    *iterator = point2;         //if 1st point already exist, wall end point can be updated
                }
                else {
                    points.push_back(point1);
                    points.push_back(point2);
                }
            }

            for (uint32_t j = 0; j < points.size(); j += 2) {
                walls.emplace_back(glm::vec4(points[j].x, points[j].y, points[j + 1].x, points[j + 1].y), abs(points[j].z));
            }

            points.clear();
        }

        //Add horizontal and vertical walls to walls vector
        for (auto& v4 : m_Diagonals) {
            uint32_t midX = (v4.x + v4.z) * 0.5f;
            uint32_t midY = (v4.y + v4.w) * 0.5f;
            uint32_t index = abs(s_MapData.map[midY * s_MapData.width + midX]);

            walls.emplace_back(v4, index);
        }

        //sort walls vector by atlas texture index
        std::sort(walls.begin(), walls.end(), [](auto& a, auto& b) {
            return a.second < b.second;
        });
    }

    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    std::vector<std::pair<uint32_t, glm::uvec4>> subranges;
    
    {
        subranges.emplace_back();

        //Create vertices and indices from walls
        uint32_t prevIndex = walls[0].second;
        uint32_t vertexCount = 0;
        uint32_t wallCount = walls.size();
        for (uint32_t i = 0; i < wallCount; i++) {
            auto& [wall, index] = walls[i];

            if (index != prevIndex) {
                glm::uvec4& previous = subranges.back().second;
                glm::uvec4 ranges(previous.y, vertices.size(), previous.w, indices.size());
                subranges.emplace_back(prevIndex, ranges);

                prevIndex = index;
                vertexCount = 0;
            }

            float dx = wall.x - wall.z;
            float dy = wall.y - wall.w;
            glm::vec3 normal = glm::normalize(glm::vec3(-dy, 0.0f, dx));

            float uvLength = std::max(abs(dx), abs(dy)) * 0.5f;

            for (uint32_t i = 0; i < 4; i++) {
                float y = (i % 2 == 0) ? 0.0f : 1.0f;
                uint32_t offset = i >= 2 ? 2 : 0;

                //position
                vertices.push_back(wall[0 + offset]);
                vertices.push_back(y);
                vertices.push_back(wall[1 + offset]);

                //normal
                vertices.push_back(normal.x);
                vertices.push_back(normal.y);
                vertices.push_back(normal.z);

                //uv
                vertices.push_back(offset * uvLength);
                vertices.push_back(y);
            }

            indices.push_back(vertexCount);
            indices.push_back(vertexCount + 1);
            indices.push_back(vertexCount + 2);
            indices.push_back(vertexCount + 2);
            indices.push_back(vertexCount + 3);
            indices.push_back(vertexCount + 1);

            vertexCount += 4;
        }

        //add last mesh
        glm::uvec4& previous = subranges.back().second;
        glm::uvec4 ranges(previous.y, vertices.size(), previous.w, indices.size());
        subranges.emplace_back(prevIndex, ranges);

        //Create vertices and indices for floor and ceiling
        const glm::vec4 floor(0.0f, 0.0f, s_MapData.width, s_MapData.height);
        const glm::vec3 normal(0.0f, 1.0f, 0.0f);
        for (uint32_t i = 0; i < 2; i++) {
            uint32_t index = (i % 2 == 0) ? 7 : 6;

            for (uint32_t j = 0; j < 4; j++) {
                uint32_t offset = j <= 1 ? j : 5 - j;
                float x = floor[(0 + offset) % 4];
                float z = floor[(1 + offset) % 4];
                //position
                vertices.push_back(x);
                vertices.push_back(i);
                vertices.push_back(z);

                //normal
                vertices.push_back(normal.x);
                vertices.push_back(normal.y);
                vertices.push_back(normal.z);

                //uv
                vertices.push_back(x);
                vertices.push_back(z);
            }

            indices.push_back(0);
            indices.push_back(1);
            indices.push_back(2);
            indices.push_back(2);
            indices.push_back(3);
            indices.push_back(1);

            glm::uvec4& previous = subranges.back().second;
            glm::uvec4 ranges(previous.y, vertices.size(), previous.w, indices.size());
            subranges.emplace_back(index, ranges);
        }
    }

    //setup shader
    auto shader = std::make_shared<Core::Shader>("3DAtlasShader.glsl");
    shader->Bind();
    shader->setInt("Texture", 0);

    uint32_t lightCount = m_Lights.size();
    glm::uvec2 atlasSize(11, 2);
    
    shader->setVec2("AtlasSize", atlasSize);
    shader->setInt("LightCount", lightCount);
    
    for (uint32_t i = 0; i < lightCount; i++) {
        std::string lightName = "PointLights[i]";
        lightName[12] = '0' + i;
        glm::vec3 pos(m_Lights[i].x, m_Lights[i].z, m_Lights[i].y);
        shader->setVec3(lightName.c_str(), pos);
    }
    
    auto textureAtlas = std::make_shared<Core::Texture2D>(GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
    textureAtlas->BindImage("wolfenstein_texture_atlas.png");

    //Create meshes from wall, floor and ceiling vertices and attach them to map model
    {
        m_Models.emplace_back();
        Core::Model& mapModel = m_Models.back();
        
        uint32_t meshCount = 0;
        for (auto& [index, ranges] : subranges) {
            std::ranges::subrange vertexData(vertices.begin() + ranges.x, vertices.begin() + ranges.y);
            std::ranges::subrange indexData(indices.begin() + ranges.z, indices.begin() + ranges.w);

            if (!vertexData.size()) {
                continue;
            }

            auto mesh = std::make_shared<Core::Mesh>();
            mesh->VAO = std::make_unique<Core::VertexArray>();
            mesh->VBO = std::make_unique<Core::VertexBuffer>(vertexData.data(), sizeof(float) * vertexData.size());
            Core::VertexBufferLayout wallLayout;

            wallLayout.Push<float>(3);
            wallLayout.Push<float>(3);
            wallLayout.Push<float>(2);
            mesh->VAO->AddBuffer(*mesh->VBO, wallLayout);

            if (indexData.size()) {
                mesh->EBO = std::make_unique<Core::ElementBuffer>(indexData.data(), indexData.size());
            }

            mapModel.Meshes.emplace_back(mesh, meshCount);

            auto mat = std::make_shared<Core::Material>();
            mat->Shader = shader;
            mat->MaterialMaps.emplace_back();
            mat->MaterialMaps.back().Texture = textureAtlas;
            mat->MaterialMaps.back().TextureIndex = 0;
            mat->Parameters.emplace_back(glm::vec2(index, 0), "AtlasOffset");
            mat->Parameters.emplace_back(glm::vec2(0.0f, 0.0f), "FlipTexture");
            mapModel.Materials.push_back(mat);

            meshCount++;
        }
    }

    //Create and set up model for static objects and enemies
    const glm::vec3 normal(0.0f, 0.0f, 1.0f);
    const uint32_t totalCount = m_StaticObjects.size() + m_Enemies.size();
    glm::vec3 scale;
    glm::vec2 index;
    for (uint32_t i = 0; i < totalCount; i++) {
        if (i < m_StaticObjects.size()) {
            scale = m_StaticObjects[i].Scale;
            index = glm::vec2(m_StaticObjects[i].AtlasIndex, 0.0f);
        } else {
            scale = m_Enemies[i - m_StaticObjects.size()].Scale;
            index.x = m_Enemies[i - m_StaticObjects.size()].AtlasIndex % atlasSize.x;
            index.y = m_Enemies[i - m_StaticObjects.size()].AtlasIndex / atlasSize.x;
        }
        m_Models.emplace_back();
        Core::Model& model = m_Models.back();

        vertices.clear();
        indices.clear();

        for (uint32_t j = 0; j < 4; j++) {
            float x = (j >= 2) ? scale.x * 0.5f : -scale.x * 0.5f;
            float y = (j % 2 == 1) ? scale.y * 0.5f : - scale.y * 0.5f;

            //position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(0.0f);

            //normal
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
            
            //uv
            vertices.push_back((j >= 2) ? 0.0f : 1.0f);
            vertices.push_back((j % 2 == 0) ? 0.0f : 1.0f);
        }

        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(2);
        indices.push_back(3);
        indices.push_back(1);

        auto mesh = std::make_shared<Core::Mesh>();
        mesh->VAO = std::make_unique<Core::VertexArray>();
        mesh->VBO = std::make_unique<Core::VertexBuffer>(vertices.data(), sizeof(float) * vertices.size());
        Core::VertexBufferLayout wallLayout;

        wallLayout.Push<float>(3);
        wallLayout.Push<float>(3);
        wallLayout.Push<float>(2);
        mesh->VAO->AddBuffer(*mesh->VBO, wallLayout);

        mesh->EBO = std::make_unique<Core::ElementBuffer>(indices.data(), indices.size());

        model.Meshes.emplace_back(mesh, 0);

        auto mat = std::make_shared<Core::Material>();
        mat->Shader = shader;
        mat->MaterialMaps.emplace_back();
        mat->MaterialMaps.back().Texture = textureAtlas;
        mat->MaterialMaps.back().TextureIndex = 0;
        mat->Parameters.emplace_back(index, "AtlasOffset");
        mat->Parameters.emplace_back(0.0f, "FlipTexture");
        model.Materials.push_back(mat);
    }
}