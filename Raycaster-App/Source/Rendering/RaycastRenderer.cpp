#include "RaycastRenderer.h"

#include <glm/gtc/round.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>

static glm::ivec2 GetBilinearOffset(uint8_t bitboard, bool decreaseX, bool decreaseY) {
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

    std::array<glm::ivec2, 8> offsets{
        glm::ivec2{ -1.0f, -1.0f },
        glm::ivec2{  1.0f, -1.0f },
        glm::ivec2{ -1.0f,  1.0f },
        glm::ivec2{  1.0f,  1.0f },

        glm::ivec2{  0.0f, -1.0f },
        glm::ivec2{ -1.0f,  0.0f },
        glm::ivec2{  1.0f,  0.0f },
        glm::ivec2{  0.0f,  1.0f },
    };
    
    for (size_t i = 0; i < 8; i++) {
        // First check all 2x2 areas, then 2x1/1x2
        size_t index = (i >= 4) * 4;

        // Index into arrays in preferred order ->
        // If possible decrease x/y, when x/y < 0.5f and vice versa
        bool flip = (i >= 4) && (decreaseX ^ decreaseY);
        bool condition[] = { !(i & 2), !(i % 2) };
        index += (decreaseY ^ condition[flip]) * 2;
        index += (flip ^ decreaseX) ^ condition[1 - flip];

        if (!(bitboard & testBitboards[index])) {
            return offsets[index];
        }
    }

    return { 0.0f, 0.0f };
}

static float LightBilinear(glm::vec2 position, const Map& map) {
    glm::ivec2 min, max;

    // Prevent sampling light map inside a wall by shifting min and max 
    {
        size_t index = map.GetIndex(position);
        bool decreaseX = (glm::fract(position.x) < 0.5f);
        bool decreaseY = (glm::fract(position.y) < 0.5f);

        // Doors need a special case :(
        if (map.HasDoor(index)) {
            auto adjacent = map.GetNeighbours(index);

            if (decreaseX && !adjacent.West || !decreaseX && !adjacent.East) {
                position.x += decreaseX ? -1.0f : 1.0f;
                index += decreaseX ? -1 : 1;
            } else if (decreaseY && !adjacent.North || !decreaseY && !adjacent.South) {
                position.y += decreaseY ? -1.0f : 1.0f;
                index += static_cast<int32_t>((decreaseY ? -1 : 1) * map.GetWidth());
            }
        }

        uint8_t mapBitboard = map.GetNeighbours(index).Bitboard;
        glm::ivec2 offset = GetBilinearOffset(mapBitboard, decreaseX, decreaseY);

        min.x = static_cast<int>(position.x) + offset.x * (offset.x < 0.0f); // Only decrease min 
        min.y = static_cast<int>(position.y) + offset.y * (offset.y < 0.0f); // Only decrease min 

        max.x = static_cast<int>(position.x) + offset.x * (offset.x > 0.0f); // Only increase max
        max.y = static_cast<int>(position.y) + offset.y * (offset.y > 0.0f); // Only increase max
    }

    // Bilinear interpolation of m_LightMap
    glm::vec2 Xmin{ map.GetLight(min.x, min.y), map.GetLight(min.x, max.y) };
    glm::vec2 Xmax{ map.GetLight(max.x, min.y), map.GetLight(max.x, max.y) };

    // Add 0.5f to min, since light is evaluated at the middle of a tile
    float mix = (position.x - (min.x + 0.5f));
    glm::vec2 y = glm::mix(Xmin, Xmax, mix);

    // Add 0.5f to min, since light is evaluated at the middle of a tile
    mix = (position.y - (min.y + 0.5f));
    return glm::clamp(glm::mix(y[0], y[1], mix), 0.0f, 1.0f);
}

void RaycastRenderer::Render(const Map& map, const Core::Camera2D& camera, const Player& player, Renderables& renderables) {
    RenderWalls(map, camera);
    RenderFloors(map, camera);
    RenderSprites(map, player, renderables);
}

void RaycastRenderer::RenderWalls(const Map& map, const Core::Camera2D& camera) {
    glm::vec2 rotation{ camera.GetDirection().x, -camera.GetDirection().y };

    //Wall casting
    for (uint32_t i = 0; i < m_RayCount; i++) {
        float cameraX = 2.0f * i / m_RayCount - 1.0f;
        glm::vec3 rayDirection = camera.GetDirection() + camera.GetPlane() * cameraX * m_AspectRatio;
        auto hit = map.CastRay(camera.GetPosition(), rayDirection);
        m_Rays[i].TexPosition.x = static_cast<float>(hit.TexturePosition) / std::numeric_limits<decltype(hit.TexturePosition)>::max();

        float wallDistance = hit.Distance;
        if (hit.Side > 1) {
            // Get the distance perpendicular to the camera plane
            glm::vec2 perpendicular{ hit.WorldPosition.x - camera.GetPosition().x, hit.WorldPosition.y - camera.GetPosition().y };
            perpendicular *= rotation;

            wallDistance = perpendicular.x + perpendicular.y;
        }

        m_Rays[i].Scale = 1.0f / wallDistance;
        if constexpr (m_SnappingEnabled) {
            // The center of a wall is @ y = 0, so scale needs to be even multiple of m_RayWidth -> round to 2.0f * m_RayWidth
            m_Rays[i].Scale = glm::floorMultiple(m_Rays[i].Scale + m_RayWidth, 2.0f * m_RayWidth);
        }

        m_Rays[i].Position.x = cameraX + 0.5f * m_RayWidth;
        m_Rays[i].Position.y = (0.5f - camera.GetPosition().z) * m_Rays[i].Scale;
        m_Rays[i].Atlasindex = hit.Material;

        m_Rays[i].Brightness = LightBilinear(hit.WorldPosition, map);

        m_ZBuffer[i] = wallDistance;
        m_Lines[i].Scale = rayDirection * wallDistance;
        m_Lines[i].Scale.y *= -1.0f;
        m_Lines[i].Posistion = camera.GetPosition();
    }
}

void RaycastRenderer::RenderFloor(bool ceiling, const Map& map, const Core::Camera2D& camera) {
    std::vector<float> visibleRanges; // Ranges in NDC

    const float reciprocalAspectRatio = 1.0f / m_AspectRatio;
    glm::vec3 rayDirection = camera.GetDirection() * reciprocalAspectRatio - camera.GetPlane();

    float sign = (ceiling ? 1.f : -1.f);
    float density = 1.0f + 2.0f * sign * (0.5f - camera.GetPosition().z);
    for (uint32_t i = 0; i < m_RayCount / 2; i++) {
        float scale = density * m_RayCount / (m_RayCount - (2.0f * i + 1)) * m_AspectRatio;
        float currentHeight = 1.0f / scale;
        float prevPos = -1.0f;

        glm::vec2 worldPosition(scale * 0.5f * rayDirection.x + camera.GetPosition().x, scale * 0.5f * -rayDirection.y + camera.GetPosition().y);
        if (worldPosition.x < 0.0f || worldPosition.x >= map.GetWidth() || worldPosition.y < 0.0f || worldPosition.y >= map.GetHeight()) {
            glm::vec2 direction(worldPosition.x + camera.GetPlane().x, worldPosition.y - camera.GetPlane().y);
            // 2 closest edges
            glm::vec2 bounds(worldPosition.x < 0.0f ? 0.0f : static_cast<float>(map.GetWidth()) - 1e-5f, worldPosition.y < 0.0f ? 0.0f : static_cast<float>(map.GetHeight()) - 1e-5f);

            // Intersections to both edges
            std::array<glm::vec2, 2> inter = {
                Algorithms::LineIntersection(glm::vec2(map.GetWidth() - bounds.x, bounds.y), bounds, direction, worldPosition, true).value_or(glm::vec2(INFINITY)),
                Algorithms::LineIntersection(bounds, glm::vec2(bounds.x, map.GetHeight() - bounds.y), direction, worldPosition, true).value_or(glm::vec2(INFINITY)),
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
                prevPos = glm::floorMultiple(prevPos + 0.5f * m_RayWidth, m_RayWidth);
            }
        }

        bool occluded = true;
        // Fixes incorrect occluded areas when walls aren't aligned to the width of a ray
        float occlusionScale = m_SnappingEnabled ? scale : density * m_RayCount / (m_RayCount - (2.0f * i));
        for (size_t j = m_RayCount; j > 0; j--) {
            // compares the current height to the height of a wall
            // m_zBuffer stores distances (half of the reciprocal height), so we compare with the reciprocal height -> (scale * 0.5f)
            if (occluded != (occlusionScale * 0.5f * reciprocalAspectRatio < m_ZBuffer[j - 1])) {
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
            auto hit = map.CastFloor(ceiling, worldPosition, camera.GetPlane(), maxDistance);

            // NDC
            float rayLength = 2.0f * hit.Distance * currentHeight;
            if (m_SnappingEnabled) {
                rayLength = glm::floorMultiple(rayLength + 0.5f * m_RayWidth, m_RayWidth);
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

                    glm::vec2 offset = camera.GetPlane() * (activeRange[0] - offsetPosition) * scale * 0.5f; // NDC to world coords
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
                    length = glm::floorMultiple(length + 0.5f * m_RayWidth, m_RayWidth);
                }

                if (length < m_RayWidth * 0.25f) {
                    break;
                }

                Floor& floor = m_Floors.emplace_back();

                floor.Position.x = 0.5f * length + position;
                floor.Position.y = sign * currentHeight * m_AspectRatio;
                floor.Length = length;

                floor.TexturePosition = worldPosition;
                floor.AtlasIndex = hit.Material;

                glm::vec2 lightingPosition = worldPosition;
                floor.BrightnessStart = LightBilinear(lightingPosition, map);

                float worldScale = length * 0.49f * scale; // NDC to world coords
                lightingPosition += glm::vec2(worldScale * camera.GetPlane().x, -worldScale * camera.GetPlane().y);

                floor.BrightnessEnd = LightBilinear(lightingPosition, map);

                position += length;
            }

            if (hit.Side > 1) {
                break;
            }

            prevPos += rayLength;
            worldPosition = hit.WorldPosition;

            if (prevPos + 1e-5f >= activeRange[1] && index > 1) {
                activeRange = glm::vec2{ visibleRanges[--index], visibleRanges[--index] };
            }
        }
        visibleRanges.clear();
    }
}

void RaycastRenderer::RenderFloors(const Map& map, const Core::Camera2D& camera) {
    m_Floors.resize(0);
    RenderFloor(true, map, camera);
    RenderFloor(false, map, camera);
}

void RaycastRenderer::RenderSprites(const Map& map, const Player& player, Renderables& renderables) {
    auto spriteObjects = renderables.GetSprites();
    auto models = renderables.GetDynamicModels();

    size_t count = spriteObjects.size();
    uint32_t rayIndex = m_RayCount;
    size_t space = m_Rays.size();
    const float reciprocalAspectRatio = 1.0f / m_AspectRatio;

    glm::mat3 matrix = glm::rotate(glm::mat3(1.0f), glm::radians(player.GetRotation() + 90.0f));

    for (size_t index = 0; index < count; index++) {
        auto& model = models[index];
        auto& sprite = spriteObjects[index];

        //3D
        glm::vec3 position3D(sprite.Position.x, sprite.Position.z, sprite.Position.y);
        model.Transform = glm::translate(glm::mat4(1.0f), position3D);
        model.Transform = glm::rotate(model.Transform, glm::radians(player.GetRotation() - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model.Transform = glm::scale(model.Transform, sprite.Scale);

        //Transform for 2D
        sprite.Position = sprite.Position - player.GetPosition();
        sprite.Position = matrix * sprite.Position;
    }

    std::sort(spriteObjects.begin(), spriteObjects.end(), [this](Sprite a, Sprite b) {
        return a.Position.y > b.Position.y;
        });

    for (size_t index = 0; index < count; index++) {
        auto& sprite = spriteObjects[index];
        glm::vec3 position = spriteObjects[index].Position;

        if (position.y < 0) {
            break;
        }

        glm::vec3 scale = sprite.Scale;
        uint32_t atlasIndex = sprite.AtlasIndex;
        bool flipTexture = sprite.FlipTexture;

        float brightness = LightBilinear({ sprite.WorldPosition.x, sprite.WorldPosition.y }, map);

        float distance = position.y;
        position.x *= -reciprocalAspectRatio / distance;
        scale /= distance;
        position.y = position.z / distance;

        if (m_SnappingEnabled) {
            position.y = glm::floorMultiple(position.y + 0.5f * m_RayWidth, m_RayWidth);
            // The center of a sprite is @ an integer multiple of ray width, so scale needs to be even multiple of m_RayWidth
            // to prevent sprite getting rendered at the middle of a floor ray -> round to 2.0f * m_RayWidth
            scale.y = glm::floorMultiple(scale.y + m_RayWidth, m_RayWidth * 2.0f);
        }

        if (scale.y < m_RayWidth * 0.25f) {
            continue;
        }

        float width = scale.x * m_RayCount * 0.5f * reciprocalAspectRatio;
        float startX = 0.5f * (m_RayCount - width + position.x * m_RayCount);
        int32_t endX = static_cast<int32_t>(startX + width);
        if (endX < 0) {
            continue;
        }
        for (size_t i = static_cast<size_t>(glm::max(startX, 0.0f)); i < endX; i++) {
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
            m_Rays[rayIndex].Brightness = brightness;

            rayIndex++;
        }
    }

    m_Rays.resize(rayIndex);
}