#include "Map.h"

std::vector<LineCollider> Map::CreateWalls() {
    std::vector<LineCollider> result;

    std::array<std::vector<glm::vec2>, 4> points;
    for (size_t startIndex = 0; startIndex < s_MapData.Size; startIndex++) {
        if (s_MapData.Map[startIndex] == 0) {
            continue;
        }

        glm::vec2 start(startIndex % s_MapData.Width, startIndex / s_MapData.Width);
        Neighbours adjacent = GetNeighbours(startIndex);

        // Horizontal or vertical wall checks
        if (s_MapData.Map[startIndex] > 0) {
            for (size_t i = 0; i < 4; i++) {
                if (adjacent[i]) {
                    continue;
                }
                
                glm::vec2 point(start.x + (i % 2 == 1), start.y + (i < 2));
                auto iterator = std::find(points[i].begin(), points[i].end(), point);

                if (iterator != points[i].end()) { //if 1st point already exist, line end point can be updated
                    iterator->x = start.x + (i > 1);
                    iterator->y = start.y + (i % 2 == 1);
                } else {
                    points[i].emplace_back(point);
                    points[i].emplace_back(start.x + (i > 1), start.y + (i % 2 == 1));
                }
            }

            continue;
        }

        glm::vec2 end = start;

        // Diagonal wall checks
        if (adjacent.Down && adjacent.Right || adjacent.Up && adjacent.Left) {
            start.x++;
            end.y++;
        } else {
            start.x++;
            start.y++;
        }

        // Fixes normals
        if (adjacent.Down) {
            result.emplace_back(end, start);
            continue;
        }

        result.emplace_back(start, end);
    }

    for (size_t i = 0; i < points.size(); i++) {
        for (size_t j = 0; j < points[i].size(); j += 2) {
            result.emplace_back(points[i][j], points[i][j + 1]);
        }
    }

    return result;
}

std::vector<Tile> Map::CreateTiles() {
    std::vector<Tile> result;
    result.reserve(s_MapData.Size);

    Tile tile;
    tile.Scale = glm::vec3(0.95f * s_MapData.Scale.x, 0.95f * s_MapData.Scale.y, 1.0f);
    tile.Posistion.z = 0.0f;

    float centreY = (s_MapData.Height - 1) / 2.0f, centreX = (s_MapData.Width - 1) / 2.0f;
    for (uint32_t i = 0; i < s_MapData.Size; i++) {
        uint32_t mapX = i % s_MapData.Width;
        uint32_t mapY = i / s_MapData.Width;

        tile.Posistion.x = (mapX - centreX) * s_MapData.Scale.x;
        tile.Posistion.y = (centreY - mapY) * s_MapData.Scale.y;

        float brightness = (s_MapData.Map[mapY * s_MapData.Width + mapX] > 0) ? 1.0f : 0.5f;

        tile.Colour = glm::vec3(brightness);
        result.push_back(tile);

        if (s_MapData.Map[mapY * s_MapData.Width + mapX] < 0) {
            Neighbours adjacent = GetNeighbours(mapY * s_MapData.Width + mapX);
            
            if (adjacent.Down && adjacent.Right || adjacent.Up && adjacent.Left) {
                tile.Rotation = adjacent.Down && !(adjacent.Up && adjacent.Left) ? 270.0f : 90.0f;
            } else {
                tile.Rotation = adjacent.Up ? 0.0f : 180.0f;
            }

            tile.Colour = glm::vec3(1.0f);
            tile.IsTriangle = true;
            
            result.push_back(tile);

            tile.IsTriangle = false;
        }
    }

    return result;
}

Core::Model Map::CreateModel(const std::span<LineCollider> walls, std::shared_ptr<Core::Texture2D> atlas, std::shared_ptr<Core::Shader> shader) {
    std::vector<std::pair<glm::vec4, uint32_t>> wallIndices;

    {
        for (auto& wall : walls) {
            uint32_t midX = static_cast<uint32_t>(wall.Position.x + 0.5f * wall.Vector.x - 1e-6f * wall.Normal.x);
            uint32_t midY = static_cast<uint32_t>(wall.Position.y + 0.5f * wall.Vector.y + 1e-6f * wall.Normal.y);
            //uint32_t index = static_cast<uint32_t>(glm::abs(s_MapData.Map[midY * s_MapData.Width + midX]));

            uint32_t index = 0;
            if (wall.Normal.x != 0) {
                index = wall.Normal.x > 0 ? 2 : 1;
            }
            if (wall.Normal.y != 0) {
            index += wall.Normal.y > 0 ? 6 : 3;
            }
        
            wallIndices.emplace_back(glm::vec4{ wall.Position.x, wall.Position.y, wall.Position.x + wall.Vector.x, wall.Position.y + wall.Vector.y }, index);
        }

        //sort walls vector by atlas texture index
        std::sort(wallIndices.begin(), wallIndices.end(), [](auto& a, auto& b) {
            return a.second < b.second;
            });
    }

    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    std::vector<std::pair<uint32_t, glm::uvec4>> subranges;

    {
        subranges.emplace_back();

        //Create vertices and indices from walls
        uint32_t prevIndex = wallIndices[0].second;
        uint32_t vertexCount = 0;
        uint32_t wallCount = static_cast<uint32_t>(wallIndices.size());
        for (uint32_t i = 0; i < wallCount; i++) {
            auto& [wall, index] = wallIndices[i];

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
        const glm::vec4 floor(0.0f, 0.0f, s_MapData.Width, s_MapData.Height);
        const glm::vec3 normal(0.0f, 1.0f, 0.0f);
        for (uint32_t i = 0; i < 2; i++) {
            uint32_t index = (i % 2 == 0) ? 7 : 6;

            for (uint32_t j = 0; j < 4; j++) {
                uint32_t offset = j <= 1 ? j : 5 - j;
                float x = floor[(0 + offset) % 4];
                float z = floor[(1 + offset) % 4];
                //position
                vertices.push_back(x);
                vertices.push_back(static_cast<float>(i));
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

    Core::Model mapModel;
    //Create meshes from wall, floor and ceiling vertices and attach them to map model
    {
        uint32_t meshCount = 0;
        for (auto& [index, ranges] : subranges) {
            std::ranges::subrange vertexData(vertices.begin() + ranges.x, vertices.begin() + ranges.y);
            std::ranges::subrange indexData(indices.begin() + ranges.z, indices.begin() + ranges.w);

            if (!vertexData.size()) {
                continue;
            }

            auto mesh = std::make_shared<Core::Mesh>();
            mesh->VAO = std::make_unique<Core::VertexArray>();
            mesh->VBO = std::make_unique<Core::VertexBuffer>(vertexData.data(), static_cast<uint32_t>(sizeof(float) * vertexData.size()));
            Core::VertexBufferLayout wallLayout;

            wallLayout.Push<float>(3);
            wallLayout.Push<float>(3);
            wallLayout.Push<float>(2);
            mesh->VAO->AddBuffer(*mesh->VBO, wallLayout);

            if (indexData.size()) {
                mesh->EBO = std::make_unique<Core::ElementBuffer>(indexData.data(), static_cast<uint32_t>(indexData.size()));
            }

            mapModel.Meshes.emplace_back(mesh, meshCount);

            auto mat = std::make_shared<Core::Material>();
            mat->Shader = shader;
            mat->MaterialMaps.emplace_back();
            mat->MaterialMaps.back().Texture = atlas;
            mat->MaterialMaps.back().TextureIndex = 0;
            mat->Parameters.emplace_back(glm::vec2(index, 0), "AtlasOffset");
            mat->Parameters.emplace_back(glm::vec2(0.0f, 0.0f), "FlipTexture");
            mapModel.Materials.push_back(mat);

            meshCount++;
        }
    }

    return mapModel;
}

Map::HitInfo Map::CastRay(glm::vec3 origin, glm::vec3 direction) {
    glm::vec3 deltaDistance = glm::abs((float)1 / direction);

    uint32_t mapX = static_cast<uint32_t>(origin.x);
    uint32_t mapY = static_cast<uint32_t>(origin.y);

    int32_t stepX = (direction.x > 0) ? 1 : -1;
    int32_t stepY = (direction.y < 0) ? 1 : -1;

    glm::vec3 sideDistance = deltaDistance;
    sideDistance.x *= (direction.x < 0) ? (origin.x - mapX) : (mapX + 1.0f - origin.x);
    sideDistance.y *= (direction.y > 0) ? (origin.y - mapY) : (mapY + 1.0f - origin.y);

    bool hit = false;
    uint8_t side = 0;
    glm::vec2 worldPosition;
    while (!hit) {
        //if diagonal, needs to be handled first, because origin may be inside diagonal
        if (s_MapData.Map[mapY * s_MapData.Width + mapX] < 0) {
            glm::vec2 point3(origin.x, origin.y);
            glm::vec2 point4 = point3;
            point3.x += direction.x;
            point3.y -= direction.y;

            glm::vec2 point1(mapX, mapY);
            glm::vec2 point2(point1);

            Neighbours adjacent = GetNeighbours(mapY * s_MapData.Width + mapX);
            if (adjacent.Down && adjacent.Right || adjacent.Up && adjacent.Left) {
                point1.x++;
                point2.y++;
            } else {
                point2.x++;
                point2.y++;
            }

            if (std::optional<glm::vec2> intersection = Algorithms::LineIntersection(point1, point2, point3, point4, true)) {
                worldPosition = intersection.value();
                sideDistance.x = intersection.value().x;
                sideDistance.y = intersection.value().y;
                sideDistance.z = 0.0f;

                side = 2;
                hit = true;

                break;
            }
        }

        if (sideDistance.x < sideDistance.y) {
            sideDistance.x += deltaDistance.x;
            mapX += stepX;
            side = 0;
        } else {
            sideDistance.y += deltaDistance.y;
            mapY += stepY;
            side = 1;
        }

        if (mapY >= s_MapData.Height || mapX >= s_MapData.Width) {
            RC_FATAL("INDEX OUT OF BOUNDS");
            break;
        }

        if (s_MapData.Map[mapY * s_MapData.Width + mapX] > 0) {
            hit = true;
        }
    }

    float wallDistance = -0.0f;
    if (side == 0) {
        wallDistance = sideDistance.x - deltaDistance.x;

        float offset = origin.y - wallDistance * direction.y;
        offset -= floor(offset);
        worldPosition = glm::vec2(mapX - stepX, mapY + offset);
    } else if (side == 1) {
        wallDistance = sideDistance.y - deltaDistance.y;

        float offset = origin.x + wallDistance * direction.x;
        offset -= floor(offset);
        worldPosition = glm::vec2(mapX + offset, mapY - stepY);
    }

    return { 
        .Distance = wallDistance, 
        .Side = side, 
        .Material = static_cast<uint8_t>(glm::abs(s_MapData.Map[mapY * s_MapData.Width + mapX])),
        .WorlPosition = worldPosition
    };
}

constexpr Map::Neighbours Map::GetNeighbours(size_t index) {
    bool left = index - 1 >= s_MapData.Size || s_MapData.Map[index - 1];
    bool down = index + s_MapData.Width >= s_MapData.Size || s_MapData.Map[index + s_MapData.Width];
    bool up = index - s_MapData.Width >= s_MapData.Size || s_MapData.Map[index - s_MapData.Width];
    bool right = index + 1 >= s_MapData.Size || s_MapData.Map[index + 1];

    return { left, down, up, right };
}