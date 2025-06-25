#include "RaycasterScene.h"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>

#include <algorithm>
#include <ranges>

void RaycasterScene::Init(){
    m_Rays.resize(m_RayCount); //should be initialized to default values
    m_Lines.resize(m_RayCount); //should be initialized to default vec3s
    
    m_Player.Position = glm::vec3((float)m_Map.GetWidth() / 2, (float)m_Map.GetHeight() / 2, 0.5f);
    m_Player.Scale = m_Map.GetScale() * 0.4f;
    m_Player.Rotation = 90.0f;

    m_Camera = std::make_unique<Core::RaycasterCamera>(m_Player.Position, m_Player.Rotation, glm::sqrt((float)m_Map.GetSize()) / 1.4f, m_Map.GetWidth(), m_Map.GetHeight());
    m_Camera3D = std::make_unique<Core::FlyCamera>(glm::vec3(m_Player.Position.x, 0.5f, m_Player.Position.y), glm::vec3(0.0f, 1.0f, 0.0f), -m_Player.Rotation, 0.0f);

    m_Lights.push_back(glm::vec3(2.5f, 3.0f, 0.75f));
    m_Lights.push_back(glm::vec3(21.5f, 3.0f, 0.75f));
    m_Lights.push_back(glm::vec3(18.5f, 18.0f, 0.75f));
    m_Lights.push_back(glm::vec3(8.5f, 6.5f, 0.75f));
    
    SpriteObject staticObject;
    staticObject.Position = glm::vec3(3.0f, 2.5f, 0.25f);
    staticObject.WorldPosition = glm::vec3(3.0f, 2.5f, 0.25f);
    staticObject.Scale = glm::vec3(0.5f, 0.5f, 0.5f);
    staticObject.AtlasIndex = 8;
    staticObject.FlipTexture = false;
    
    m_StaticObjects.push_back(staticObject);
    staticObject.Position.x = 2.5f;
    m_StaticObjects.push_back(staticObject);

    staticObject.AtlasIndex = 10;
    for (const glm::vec3& light : m_Lights) {
        staticObject.Position = light;
        staticObject.WorldPosition = light;
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

    for (uint32_t i = 0; i < m_Map.GetSize(); i++) {
        m_EnemyMap[i] = m_Map[i];
    }

    m_Map.CalculateLightMap(m_Lights);
    m_Tiles = m_Map.CreateTiles();
    m_Walls = m_Map.CreateWalls();
    InitModels();
    
    m_SpriteObjects.resize(m_StaticObjects.size() + m_Enemies.size());

    Tile tile;
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
        CastFloors();
        RenderSprites();
    }
}

void RaycasterScene::CastRays() {
    float planeAngle = glm::radians(m_Player.Rotation + 90.0f);
    glm::vec2 rotation{ glm::sin(planeAngle), glm::cos(planeAngle) };

    //Wall casting
    for (uint32_t i = 0; i < m_RayCount; i++) {
        float cameraX = 2 * i / float(m_RayCount) - 1;
        glm::vec3 rayDirection = m_Camera->GetDirection() + m_Camera->GetPlane() * cameraX;
        auto hit = m_Map.CastRay(m_Player.Position, rayDirection);

        float wallDistance = 0.0f;
        if (hit.Side == 0) {
            wallDistance = hit.Distance;

            m_Rays[i].TexPosition.x = hit.WorlPosition.y - glm::floor(hit.WorlPosition.y);
        }
        else if (hit.Side == 1) {
            wallDistance = hit.Distance;

            m_Rays[i].TexPosition.x = hit.WorlPosition.x - glm::floor(hit.WorlPosition.x);
        } else {
            m_Rays[i].TexPosition.x = hit.WorlPosition.x;
            
            // Get the distance perpendicular to the camera plane
            glm::vec2 perpendicular{ hit.WorlPosition.x - m_Player.Position.x, hit.WorlPosition.y - m_Player.Position.y };
            perpendicular *= rotation;

            wallDistance = perpendicular.x + perpendicular.y;
        }

        m_Rays[i].Scale = 1.0f / wallDistance;
        if (m_SnappingEnabled) {
            // The center of a wall is @ y = 0, so scale needs to be even multiple of m_RayWidth -> round to 2.0f * m_RayWidth
            m_Rays[i].Scale = glm::round(0.25f * m_Rays[i].Scale * m_RayCount) * 2.0f * m_RayWidth;
        }
        
        m_Rays[i].Position.x = cameraX + 0.5f * m_RayWidth;
        m_Rays[i].Atlasindex = hit.Material;
        
        m_Rays[i].Brightness = LightBilinear(hit.WorlPosition);

        m_ZBuffer[i] = wallDistance;
        m_Lines[i].Scale = rayDirection * wallDistance * m_Map.GetScale();
    }
}

void RaycasterScene::CastFloors() {
    size_t floorIndex = 0;
    std::vector<float> visibleRanges; // Ranges in NDC

    glm::vec3 rayDirection = m_Camera->GetDirection() - m_Camera->GetPlane();
    for (uint32_t i = 0; i < m_RayCount / 2; i++) {
        float scale = m_RayCount / (m_RayCount - (2.0f * i + 1));
        float currentHeight = 1.0f / scale;
        float prevPos = -1.0f;

        glm::vec2 worldPosition(scale * 0.5f * rayDirection.x + m_Player.Position.x, scale * 0.5f * -rayDirection.y + m_Player.Position.y);
        if (worldPosition.x < 0.0f || worldPosition.x >= m_Map.GetWidth() || worldPosition.y < 0.0f || worldPosition.y >= m_Map.GetHeight()) {
            glm::vec2 direction(worldPosition.x + m_Camera->GetPlane().x, worldPosition.y - m_Camera->GetPlane().y);
            // 2 closest edges
            glm::vec2 bounds(worldPosition.x < 0.0f ? 0.0f : static_cast<float>(m_Map.GetWidth()) - 1e-5f, worldPosition.y < 0.0f ? 0.0f : static_cast<float>(m_Map.GetHeight()) - 1e-5f);

            // Intersections to both edges
            std::array<glm::vec2, 2> inter = {
                Algorithms::LineIntersection(glm::vec2(m_Map.GetWidth() - bounds.x, bounds.y), bounds, direction, worldPosition, true).value_or(glm::vec2(INFINITY)),
                Algorithms::LineIntersection(bounds, glm::vec2(bounds.x, m_Map.GetHeight() - bounds.y), direction, worldPosition, true).value_or(glm::vec2(INFINITY)),
            };

            glm::vec2 distance(glm::distance(worldPosition, inter[0]), glm::distance(worldPosition, inter[1]));

            glm::length_t index = distance[0] < distance[1] ? 0 : 1;

            if (distance[index] == INFINITY) {
                continue;
            }

            worldPosition = inter[index];

            float lenght = distance[index] * currentHeight;
            prevPos = lenght * 2.0f - 1.0f;
            if (m_SnappingEnabled) {
                prevPos = glm::round(prevPos * m_RayCount * 0.5f) * m_RayWidth;
            }
        }

        bool occluded = true;
        // Fixes incorrect occluded areas when walls aren't aligned to the width of a ray
        float occlusionScale = m_SnappingEnabled ? scale : m_RayCount / (m_RayCount - (2.0f * i));
        for (size_t j = m_RayCount; j > 0; j--) {
            // compares the current height to the height of a wall
            // m_zBuffer stores distances (half of the reciprocal height), so we compare with the reciprocal height -> (scale * 0.5f)
            if (occluded != (occlusionScale * 0.5f < m_ZBuffer[j - 1])) {
                continue;
            }
        
            // X position in NDC
            visibleRanges.emplace_back(2.0f * j / static_cast<float>(m_RayCount) - 1.0f);
        
            occluded = !occluded;
        }

        size_t index = visibleRanges.size();
        if (!index) {
            break; // Every ceiling after this will be occluded by walls
        }

        if (index % 2 == 1) {
            visibleRanges.emplace_back(prevPos);
            index++;
        }

        float maxPos = visibleRanges.front();

        float minPos = glm::max(visibleRanges[--index], prevPos);
        glm::vec2 activeRange{ minPos, visibleRanges[--index] };
        
        while (prevPos <= maxPos) {
            float maxDistance = (2.0f - prevPos) * scale * 0.5f;
            auto hit = m_Map.CastFloors(worldPosition, m_Camera->GetPlane(), maxDistance);

            // NDC
            float rayLength = 2.0f * hit.Distance * currentHeight;
            if (m_SnappingEnabled) {
                rayLength = glm::round(rayLength * m_RayCount * 0.5f) * m_RayWidth;
            }
            float maxX = glm::min(rayLength + prevPos, maxPos);

            float position = prevPos;
            float offsetPosition = prevPos;
            while (maxX > position) {
                float length = rayLength - (position - prevPos);
                if (position + length < activeRange[0]) {
                    break;
                }

                if (position < activeRange[0]) {
                    float diffrence = activeRange[0] - position;

                    if (length - diffrence < m_RayWidth * 0.25f) {
                        break;
                    }

                    glm::vec2 offset = m_Camera->GetPlane() * (activeRange[0] - offsetPosition) * scale * 0.5f; // NDC to world coords
                    worldPosition.x += offset.x;
                    worldPosition.y -= offset.y;

                    length -= diffrence;
                    position = activeRange[0];
                    offsetPosition = position;
                }

                if (position + length >= activeRange[1]) {
                    length = activeRange[1] - position;

                    if (index > 1) {
                        activeRange = glm::vec2{ visibleRanges[--index], visibleRanges[--index] };
                    }
                }

                if (m_SnappingEnabled) {
                    length = glm::round(length * m_RayCount * 0.5f) * m_RayWidth;
                }
                
                if (length < m_RayWidth * 0.25f) {
                    break;
                }

                if (floorIndex == m_Floors.size()) {
                    m_Floors.emplace_back();
                }
                Floor& floor = m_Floors[floorIndex++];

                floor.Position.x = 0.5f * length + position;
                floor.Position.y = currentHeight;
                floor.Length = length;

                floor.TexturePosition = worldPosition;
                floor.BottomAtlasIndex = hit.BottomMaterial;
                floor.TopAtlasIndex = hit.TopMaterial;
                
                glm::vec2 lightingPosition = worldPosition;
                floor.BrightnessStart = LightBilinear(lightingPosition);

                float worldScale = length * 0.49f * scale; // NDC to world coords
                lightingPosition += glm::vec2(worldScale * m_Camera->GetPlane().x, -worldScale * m_Camera->GetPlane().y);

                floor.BrightnessEnd = LightBilinear(lightingPosition);

                position += length;
            }
            
            if (hit.Side > 1) {
                break;
            }

            prevPos += rayLength;
            worldPosition = hit.WorlPosition; 

            if (prevPos + 1e-5f >= activeRange[1] && index > 1) {
                activeRange = glm::vec2{ visibleRanges[--index], visibleRanges[--index] };
            }
        }

        visibleRanges.clear();
    }

    m_Floors.resize(floorIndex);
}

void RaycasterScene::RenderSprites() {
    uint32_t count = m_SpriteObjects.size();
    uint32_t rayIndex = m_RayCount;
    uint32_t space = m_Rays.size();

    glm::mat3 matrix = glm::rotate(glm::mat3(1.0f), glm::radians(m_Player.Rotation + 90.0f));

    for (uint32_t index = 0; index < count; index++) {
        //3D
        glm::vec3 position3D(m_SpriteObjects[index].Position.x, m_SpriteObjects[index].Position.z, m_SpriteObjects[index].Position.y);
        m_Models[index + 1].Transform = glm::translate(glm::mat4(1.0f), position3D);
        m_Models[index + 1].Transform = glm::rotate(m_Models[index + 1].Transform, glm::radians(m_Player.Rotation - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        
        //Transform for 2D
        m_SpriteObjects[index].Position = m_SpriteObjects[index].Position - m_Player.Position;
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

        float brightness = LightBilinear({ m_SpriteObjects[index].WorldPosition.x, m_SpriteObjects[index].WorldPosition.y });
        
        float distance = position.y;
        position.x *= -1.0f / distance;
        scale /= distance;
        position.y = position.z / distance;
        
        if (m_SnappingEnabled) {
            position.y = glm::round(position.y * m_RayCount * 0.5f) * m_RayWidth;
            // The center of a sprite is @ an integer multiple of ray width, so scale needs to be even multiple of m_RayWidth
            // to prevent sprite getting rendered at the middle of a floor ray -> round to 2.0f * m_RayWidth
            scale.y = glm::round(scale.y * m_RayCount * 0.25f) * m_RayWidth * 2.0f;
        }
        
        if (scale.y < m_RayWidth * 0.25f) {
            continue;
        }

        float width = scale.x * m_RayCount * 0.5f;
        float startX = 0.5f * (m_RayCount - width + position.x * m_RayCount);
        int32_t endX = startX + width;
        if (endX < 0) {
            continue;
        }
        for (size_t i = glm::max(startX, 0.0f); i < endX; i++) {
            if (i >= m_RayCount || m_ZBuffer[i] < distance) {
                continue;
            }

            if (rayIndex >= space) {
                Ray r;
                m_Rays.push_back(r);
            }

            position.x = (i + 0.5f) * m_RayWidth - 1.0f;
            float texturePosition = std::max((i - startX) / width, 0.0f);

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
    size_t spriteIndex = m_SpriteObjects.size() - count;
    uint32_t modelIndex = m_Models.size() - count;
    
    for (uint32_t i = 0; i < count; i++) {
        Enemy& enemy = m_Enemies[i];
        uint32_t atlasOffset = 0;

        uint32_t mapIndex = (uint32_t)enemy.Position.y * m_Map.GetWidth() + (uint32_t)enemy.Position.x;
        m_EnemyMap[mapIndex] = m_Map[mapIndex];
        
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

                lineOfSight &= m_Map.LineOfSight(enemyPos, playerPos);
            }

            if (!lineOfSight) {
                enemyPos.x = enemy.Position.x;
                enemyPos.y = enemy.Position.y;

                movementVector = Algorithms::AStar(enemyPos, playerPos, m_EnemyMap, m_Map.GetWidth(), m_Map.GetHeight());
                movementVector += 0.5f; //Pathfind to tile centre
            }

            movementVector.x -= enemy.Position.x;
            movementVector.y -= enemy.Position.y;
            movementVector = glm::normalize(movementVector);
            movementVector *= deltaTime * enemy.Speed;

            enemy.Position.x += movementVector.x;
            enemy.Position.y += movementVector.y;
        }

        m_EnemyMap[(uint32_t)enemy.Position.y * m_Map.GetWidth() + (uint32_t)enemy.Position.x] = true;
        m_SpriteObjects[spriteIndex + i].Position = enemy.Position;
        m_SpriteObjects[spriteIndex + i].WorldPosition = enemy.Position;
        m_SpriteObjects[spriteIndex + i].Scale = enemy.Scale;
        m_SpriteObjects[spriteIndex + i].AtlasIndex = enemy.AtlasIndex + atlasOffset;
        bool flip = (uint32_t)enemy.Tick % 2 == 0;
        m_SpriteObjects[spriteIndex + i].FlipTexture = flip;

        //Update on 2D-layer
        uint32_t centreY = m_Map.GetHeight() * 0.5f, centreX = m_Map.GetWidth() * 0.5f;
        m_Tiles[tileIndex - i].Posistion.x = (enemy.Position.x - centreX) * m_Map.GetScale().x;
        m_Tiles[tileIndex - i].Posistion.y = (centreY - enemy.Position.y) * m_Map.GetScale().y;

        //Update on 3D-layer
        uint32_t atlasWidth = 11;
        glm::vec2 index = glm::vec2((enemy.AtlasIndex + atlasOffset) % atlasWidth, (enemy.AtlasIndex + atlasOffset) / atlasWidth);
        m_Models[modelIndex + i].Materials.front()->Parameters.back().Value = glm::vec2(flip ? 0.0f : 1.0f, 0.0f);
        m_Models[modelIndex + i].Materials.front()->Parameters.front().Value = index;
    }
}

void RaycasterScene::InitModels() {
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

    auto textureAtlas = std::make_shared<Core::Texture2D>(Core::Texture2D::WrapMode::Repeat, Core::Texture2D::WrapMode::Repeat, Core::Texture2D::Filter::Nearest, Core::Texture2D::Filter::Nearest);
    textureAtlas->BindImage("wolfenstein_texture_atlas.png");
    
    // Map model (walls and ceilings)
    m_Models.push_back(m_Map.CreateModel(m_Walls, textureAtlas, shader));

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

        std::vector<float> vertices;
        std::vector<uint32_t> indices;

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

static glm::vec2 GetBilinearOffset(uint8_t bitboard, glm::vec2 position) {
    /*
    Bitboard        NAND(bitboard, test bitboard)    Result offsets
    0 = empty                                              x  y
        empty                                        -- = -1 -1
    ? = or                                           00 =  0  0
        filled                                       ++ =  1  1

    00? 00? ???         !(u8 & 110 10 000)           --
    ?00 ?00 ???         !(u8 & 011 01 000)           +-
    ??? 00? 00?         !(u8 & 000 10 110)           -+
    ??? ?00 ?00         !(u8 & 000 01 011)           ++

    ?0? ?0? ???         !(u8 & 010 00 000)           0-
    ??? 00? ???         !(u8 & 000 10 000)           -0
    ??? ?00 ???         !(u8 & 000 01 000)           +0
    ??? ?0? ?0?         !(u8 & 000 00 010)           0+
    */

    std::array<uint8_t, 8> testBitboards{
        0b11010000,
        0b01101000,
        0b00010110,
        0b00001011,

        0b01000000,
        0b00010000,
        0b00001000,
        0b00000010,
    };

    std::array<glm::vec2, 8> offsets{
        glm::vec2{ -1.0f, -1.0f },
        glm::vec2{  1.0f, -1.0f },
        glm::vec2{ -1.0f,  1.0f },
        glm::vec2{  1.0f,  1.0f },

        glm::vec2{  0.0f, -1.0f },
        glm::vec2{ -1.0f,  0.0f },
        glm::vec2{  1.0f,  0.0f },
        glm::vec2{  0.0f,  1.0f },
    };

    position = glm::fract(position);
    bool decreaseY = (position.y < 0.5f);
    bool decreaseX = (position.x < 0.5f);

    for (size_t i = 0; i < 8; i++) {
        // First check all 2x2 areas, then 2x1/1x2
        size_t index = (i >= 4) * 4;

        // Index into arrays in preferred order ->
        // If possible decrease x/y, when x/y < 0.5f and vice versa
        index += (decreaseY ^ !(i & 2)) * 2;
        index += decreaseX ^ !(i % 2);

        if (!(bitboard & testBitboards[index])) {
            return offsets[index];
        }
    }

    return { 0.0f, 0.0f };
}

float RaycasterScene::LightBilinear(glm::vec2 position) {
    // Bilinear interpolation of m_LightMap
    glm::ivec2 min = position;
    glm::ivec2 max = min;

    // Prevent sampling light map inside a wall by shifting min and max 
    {
        uint8_t mapBitboard = m_Map.GetNeighbours(min.y * m_Map.GetWidth() + min.x).Bitboard;

        glm::vec2 offset = GetBilinearOffset(mapBitboard, position);

        min.x += offset.x * (offset.x < 0.0f); // Only decrease min 
        min.y += offset.y * (offset.y < 0.0f); // Only decrease min 

        max.x += offset.x * (offset.x > 0.0f); // Only increase max
        max.y += offset.y * (offset.y > 0.0f); // Only increase max
    }

    glm::vec2 Xmin{ m_Map.GetLight(min.x, min.y), m_Map.GetLight(min.x, max.y) };
    glm::vec2 Xmax{ m_Map.GetLight(max.x, min.y), m_Map.GetLight(max.x, max.y) };

    // Add 0.5f to min, since light is evaluated at the middle of a tile
    float mix = glm::clamp(position.x - (min.x + 0.5f), 0.0f, 1.0f);
    glm::vec2 y = glm::mix(Xmin, Xmax, mix);

    // Add 0.5f to min, since light is evaluated at the middle of a tile
    mix = glm::clamp(position.y - (min.y + 0.5f), 0.0f, 1.0f);
    return glm::mix(y[0], y[1], mix);
}