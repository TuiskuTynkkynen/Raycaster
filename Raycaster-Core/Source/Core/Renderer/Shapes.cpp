#include "Shapes.h"

#include "Core/Debug/Debug.h"

namespace Core {
    std::vector<glm::vec2> Shapes::TrianglePositions(const glm::vec3& angles, float scale) {
        std::vector<glm::vec2> result;
        result.reserve(3);

        if (angles[0] + angles[1] + angles[2] != 180.0f) {
            RC_WARN("Shapes::Triangle angles must add up to 180");
            return result;
        }
        if (scale == 0.0f) {
            RC_WARN("Shapes::Triangle must have a non 0 scale");
            return result;
        }

        float len = glm::sin(glm::radians(angles[0])) / glm::sin(glm::radians(angles[1]));
        glm::vec2 side = glm::vec2(glm::cos(glm::radians(angles[2])), glm::sin(glm::radians(angles[2]))) * len;
        float sign = 1.0f - 2.0f * (side.x > 0.f);

        float width = glm::max(glm::max(side.x, glm::abs(1.f - side.x)), 1.0f);
        scale /= glm::max(width, side.y);
        
        result.emplace_back((sign * 0.5f * width + side.x - sign * 0.5f - 0.5f) * scale, 0.5f * side.y * scale);
        result.emplace_back(sign * (0.5f * width - 1.f) * scale, -0.5f * scale * side.y);
        result.emplace_back(sign * 0.5f * width * scale, -0.5f * scale * side.y);
        
        return result;
    }

    Shapes::ShapeError Shapes::Triangle(const glm::vec3& angles, float scale, std::vector<Shapes::Vertex>& vertices, std::vector<uint32_t>& indices) {
        if (vertices.capacity() - vertices.size() < 3) {
            return ShapeError::VertexOverflow;
        }
        if (indices.capacity() - indices.size() < 3) {
            return ShapeError::IndexOverflow;
        }
        if (angles[0] + angles[1] + angles[2] != 180.0f) {
            RC_WARN("Shapes::Triangle angles must add up to 180");
            return ShapeError::InvalidParameters;
        } 
        if(scale == 0.0f) {
            RC_WARN("Shapes::Triangle must have a non 0 scale");
            return ShapeError::InvalidParameters;
        }

        float len = glm::sin(glm::radians(angles[0])) / glm::sin(glm::radians(angles[1]));
        glm::vec2 side = glm::vec2(glm::cos(glm::radians(angles[2])), glm::sin(glm::radians(angles[2]))) * len;
        float sign = 1.0f - 2.0f * (side.x > 0.f);

        float width = glm::max(glm::max(side.x, glm::abs(1.f - side.x)), 1.0f);
        scale /= glm::max(width, side.y);

        size_t vertOffset = vertices.size();
        vertices.resize(vertOffset + 3);
        
        vertices[vertOffset + 0].Position = { (sign * 0.5f * width + side.x - sign * 0.5f - 0.5f) * scale, 0.5f * side.y * scale, 0.0f };
        vertices[vertOffset + 1].Position = { sign * (0.5f * width - 1.f) * scale, -0.5f * scale * side.y, 0.0f };
        vertices[vertOffset + 2].Position = { sign * 0.5f * width * scale, -0.5f * scale * side.y, 0.0f };
        
        glm::vec2 texureSize(width, side.y);
        texureSize /= glm::max(width, side.y);

        vertices[vertOffset + 0].TextureCoords = { (side.x > 0.f) * side.x / width * texureSize.x, 0.5f + texureSize.y * 0.5f };
        vertices[vertOffset + 1].TextureCoords = { 0.5f + sign * (0.5f * texureSize.x - texureSize.x / width), 0.5f - texureSize.y * 0.5f};
        vertices[vertOffset + 2].TextureCoords = { 0.5f * (sign + texureSize.x), 0.5f - texureSize.y * 0.5f};
        
        size_t indOffset = indices.size();
        indices.resize(indOffset + 3);

        indices[indOffset + 0] = vertOffset + 0;
        indices[indOffset + 1] = vertOffset + 1;
        indices[indOffset + 2] = vertOffset + 2;

        return ShapeError::None;
    }

    std::vector<glm::vec2> Shapes::QuadPositions(const glm::vec2& size){
        return {
            0.5f * size,
            { 0.5f * size.x, -0.5f * size.y},
            {-0.5f * size.x,  0.5f * size.y},
            { 0.5f * size.x, -0.5f * size.y},
            {-0.5f * size.x,  0.5f * size.y},
            -0.5f * size,
        };
    }

    Shapes::ShapeError Shapes::Quad(const glm::vec2& size, std::vector<Shapes::Vertex>& vertices, std::vector<uint32_t>& indices) {
        if (vertices.capacity() - vertices.size() < 4) {
            return ShapeError::VertexOverflow;
        }
        if (indices.capacity() - indices.size() < 6) {
            return ShapeError::IndexOverflow;
        }

        if (size.x * size.y == 0.0f) {
            RC_WARN("Shapes::QuadEdge must have a non 0 size");
            return ShapeError::InvalidParameters;
        }

        size_t vertOffset = vertices.size();
        vertices.resize(vertOffset + 4);

        vertices[vertOffset + 0].Position = { 0.5f * size.x, 0.5f * size.y, 0.0f };
        vertices[vertOffset + 1].Position = { 0.5f * size.x, -0.5f * size.y, 0.0f };
        vertices[vertOffset + 2].Position = { -0.5f * size.x,  0.5f * size.y, 0.0f };
        vertices[vertOffset + 3].Position = { -0.5f * size.x, -0.5f * size.y, 0.0f };

        vertices[vertOffset + 0].TextureCoords = { 1.0f, 1.0f };
        vertices[vertOffset + 1].TextureCoords = { 1.0f, 0.0f };
        vertices[vertOffset + 2].TextureCoords = { 0.0f, 1.0f };
        vertices[vertOffset + 3].TextureCoords = { 0.0f, 0.0f };

        size_t indOffset = indices.size();
        indices.resize(indOffset + 6);

        indices[indOffset + 0] = vertOffset + 0;
        indices[indOffset + 1] = vertOffset + 1;
        indices[indOffset + 2] = vertOffset + 2;
        indices[indOffset + 3] = vertOffset + 1;
        indices[indOffset + 4] = vertOffset + 2;
        indices[indOffset + 5] = vertOffset + 3;

        return ShapeError::None;
    }

    std::vector<glm::vec2> Shapes::QuadEdgePositions(const glm::vec2& size, float thickness) {
        std::vector<glm::vec2> result;

        if (size.x * size.y == 0.0f) {
            return result;
        }

        result.resize(24);
        for (uint32_t i = 0; i < 4; i++) {
            float sign = 1.f - i % 2 * 2.f;
            float foo = i >= 2;

            size_t dimension = i < 2;

            result[6 * i + 0][dimension] = size[dimension] * 0.5f * sign;
            result[6 * i + 1][dimension] = size[dimension] * 0.5f * sign;
            result[6 * i + 2][dimension] = size[dimension] * 0.5f * sign - thickness * sign;
            result[6 * i + 3][dimension] = size[dimension] * 0.5f * sign;
            result[6 * i + 4][dimension] = size[dimension] * 0.5f * sign - thickness * sign;
            result[6 * i + 5][dimension] = size[dimension] * 0.5f * sign - thickness * sign;

            result[6 * i + 0][1 - dimension] = size[1 - dimension] * 0.5f * sign - thickness * sign * foo;
            result[6 * i + 1][1 - dimension] = size[1 - dimension] * -0.5f * sign + thickness * sign * foo;
            result[6 * i + 2][1 - dimension] = size[1 - dimension] * 0.5f * sign - thickness * sign * foo;
            result[6 * i + 3][1 - dimension] = size[1 - dimension] * -0.5f * sign + thickness * sign * foo;
            result[6 * i + 4][1 - dimension] = size[1 - dimension] * 0.5f * sign - thickness * sign * foo;
            result[6 * i + 5][1 - dimension] = size[1 - dimension] * -0.5f * sign + thickness * sign * foo;
        }

        return result;
    }

    Shapes::ShapeError Shapes::QuadEdge(const glm::vec2& size, float thickness, std::vector<Shapes::Vertex>& vertices, std::vector<uint32_t>& indices) {
        if (vertices.capacity() - vertices.size() < 12) {
            return ShapeError::VertexOverflow;
        }
        if (vertices.capacity() - vertices.size() < 12 || indices.capacity() - indices.size() < 24) {
            return ShapeError::IndexOverflow;
        }
        if (size.x * size.y == 0.0f) {
            RC_WARN("Shapes::QuadEdge must have a non 0 size");
            return ShapeError::InvalidParameters;
        }

        size_t vertOffset = vertices.size();
        vertices.resize(vertOffset + 12);
        size_t indOffset = indices.size();
        indices.resize(indOffset + 24);
        
        //Top and bottom edges
        for (uint32_t i = 0; i < 2; i++) {
            float sign = 1.f - i % 2 * 2.f;

            vertices[vertOffset + 4 * i + 0].Position.x = size.x * -0.5f * sign;
            vertices[vertOffset + 4 * i + 0].Position.y = size.y * -0.5f * sign;
            vertices[vertOffset + 4 * i + 0].TextureCoords = { i, i };

            vertices[vertOffset + 4 * i + 1].Position.x = size.x * 0.5f * sign;
            vertices[vertOffset + 4 * i + 1].Position.y = size.y * -0.5f * sign;
            vertices[vertOffset + 4 * i + 1].TextureCoords = { 1 - i, i };

            vertices[vertOffset + 4 * i + 2].Position.x = size.x * -0.5f * sign;
            vertices[vertOffset + 4 * i + 2].Position.y = size.y * -0.5f * sign + thickness * sign;
            vertices[vertOffset + 4 * i + 2].TextureCoords = { i, i + thickness * sign };

            vertices[vertOffset + 4 * i + 3].Position.x = size.x * 0.5f * sign;
            vertices[vertOffset + 4 * i + 3].Position.y = size.y * -0.5f * sign + thickness * sign;
            vertices[vertOffset + 4 * i + 3].TextureCoords = { 1 - i, i + thickness * sign };
        }

        for (uint32_t i = 0; i < 2; i++) {
            indices[indOffset + i * 6 + 0] = vertOffset + i * 4 + 0;
            indices[indOffset + i * 6 + 1] = vertOffset + i * 4 + 1;
            indices[indOffset + i * 6 + 2] = vertOffset + i * 4 + 2;
            indices[indOffset + i * 6 + 3] = vertOffset + i * 4 + 1;
            indices[indOffset + i * 6 + 4] = vertOffset + i * 4 + 2;
            indices[indOffset + i * 6 + 5] = vertOffset + i * 4 + 3;
        }
        indOffset += 12;

        //Left and right edges
        for (uint32_t i = 0; i < 2; i++) {
            float sign = 1.f - i % 2 * 2.f;

            vertices[vertOffset + 8 + 2 * i + 0].TextureCoords = { i + thickness * sign, i + thickness * sign };
            vertices[vertOffset + 8 + 2 * i + 0].Position.x = size.x * -0.5f * sign + thickness * sign;
            vertices[vertOffset + 8 + 2 * i + 0].Position.y = size.y * -0.5f * sign + thickness * sign;

            vertices[vertOffset + 8 + 2 * i + 1].TextureCoords = { i + thickness * sign, 1.0f - i - thickness * sign };
            vertices[vertOffset + 8 + 2 * i + 1].Position.x = size.x * -0.5f * sign + thickness * sign;
            vertices[vertOffset + 8 + 2 * i + 1].Position.y = size.y * 0.5f * sign - thickness * sign;
        }

        for (uint32_t i = 0; i < 2; i++) {
            indices[indOffset + i * 6 + 0] = vertOffset + i * 4 + 2;
            indices[indOffset + i * 6 + 1] = vertOffset + (1 - i) * 4 + 3;
            indices[indOffset + i * 6 + 3] = vertOffset + (1 - i) * 4 + 3;

            indices[indOffset + i * 6 + 2] = vertOffset + 8 + i * 2 + 0;
            indices[indOffset + i * 6 + 4] = vertOffset + 8 + i * 2 + 0;
            indices[indOffset + i * 6 + 5] = vertOffset + 8 + i * 2 + 1;
        }

        return ShapeError::None;
    }

    std::vector<glm::vec2> Shapes::RoundedQuadPositions(const glm::vec2& size, float roundness, uint32_t segments) {
        //Attribution: github.com/raysan5/raylib/src/rshapes.c

        if (roundness * segments <= 0.f) {
            return QuadPositions(size);
        }

        std::vector<glm::vec2> result;
        if (size.x * size.y == 0.0f) {
            RC_WARN("Shapes::RoundedQuad must have a non 0 size");
            return result;
        }

        roundness = std::min(roundness, 1.f) * 0.5f;
        const float radius = std::max(abs(size.x * roundness), abs(size.y * roundness));
        if (radius == 0.f) {
            return result;
        }
        
        result.reserve(segments * 12 + 18);
        const std::vector<glm::vec2> points = {
            glm::vec2(-size.x * 0.5f, size.y * 0.5f - radius),
            glm::vec2(radius - size.x * 0.5f, size.y * 0.5f),
            glm::vec2(size.x * 0.5f - radius, size.y * 0.5f),
            glm::vec2(size.x * 0.5f, size.y * 0.5f - radius),
            glm::vec2(size.x * 0.5f, radius - size.y * 0.5f),
            glm::vec2(size.x * 0.5f - radius, -size.y * 0.5f),
            glm::vec2(radius - size.x * 0.5f, -size.y * 0.5f),
            glm::vec2(-size.x * 0.5f, radius - size.y * 0.5f),

            glm::vec2(radius - size.x * 0.5f, size.y * 0.5f - radius),
            glm::vec2(size.x * 0.5f - radius, size.y * 0.5f - radius),
            glm::vec2(size.x * 0.5f - radius, radius - size.y * 0.5f),
            glm::vec2(radius - size.x * 0.5f, radius - size.y * 0.5f),
        };

        const float step = glm::radians(90.0f / segments);

        for (size_t k = 0; k < 4; k++) {
            if (segments == 1) {
                result.emplace_back(points[k + 8]);
                result.emplace_back(points[k * 2]);
                result.emplace_back(points[k * 2 + 1]);
            
                continue;
            }

            float angle = glm::radians(90.f - k * 90.f);
            const glm::vec2& center = points[8 + k];

            for (int i = 0; i < segments; i++) {
                result.emplace_back(center);
                result.emplace_back(center.x + glm::cos(angle) * radius, center.y + glm::sin(angle) * radius);

                angle += step;
                result.emplace_back(center.x + glm::cos(angle) * radius, center.y + glm::sin(angle) * radius);
            }
        }

        for (size_t i = 0; i < 3; i += 2) {
            result.emplace_back(points[1 + i * 2]);
            result.emplace_back(points[2 + i * 2]);
            result.emplace_back(points[8 + i]);
            result.emplace_back(points[2 + i * 2]);
            result.emplace_back(points[8 + i]);
            result.emplace_back(points[9 + i]);
        }

        result.emplace_back(points[3]);
        result.emplace_back(points[0]);
        result.emplace_back(points[4]);
        result.emplace_back(points[0]);
        result.emplace_back(points[4]);
        result.emplace_back(points[7]);

        return result;
    }
    
    Shapes::ShapeError Shapes::RoundedQuad(const glm::vec2& size, float roundness, uint32_t segments, std::vector<Shapes::Vertex>& vertices, std::vector<uint32_t>& indices) {
        //Attribution: github.com/raysan5/raylib/src/rshapes.c
        if (roundness * segments <= 0.f) {
            return Quad(size, vertices, indices);
        }
        if (vertices.capacity() - vertices.size() < (segments - 1) * 4 + 12) {
            return ShapeError::VertexOverflow;
        }
        if (indices.capacity() - indices.size() < segments * 12 + 18) {
            return ShapeError::IndexOverflow;
        }
        if (size.x * size.y == 0.0f) {
            RC_WARN("Shapes::RoundedQuad must have a non 0 size");
            return ShapeError::InvalidParameters;
        }

        roundness = std::min(roundness, 1.f) * 0.5f;
        const float radius = std::max(abs(size.x * roundness), abs(size.y * roundness));
        if (radius == 0.f) {
            return ShapeError::InvalidParameters;
        }

        const std::vector<glm::vec2> points = {
            glm::vec2(-size.x * 0.5f, size.y * 0.5f - radius),
            glm::vec2(radius - size.x * 0.5f, size.y * 0.5f),
            glm::vec2(size.x * 0.5f - radius, size.y * 0.5f),
            glm::vec2(size.x * 0.5f, size.y * 0.5f - radius),
            glm::vec2(size.x * 0.5f, radius - size.y * 0.5f),
            glm::vec2(size.x * 0.5f - radius, -size.y * 0.5f),
            glm::vec2(radius - size.x * 0.5f, -size.y * 0.5f),
            glm::vec2(-size.x * 0.5f, radius - size.y * 0.5f),

            glm::vec2(radius - size.x * 0.5f, size.y * 0.5f - radius),
            glm::vec2(size.x * 0.5f - radius, size.y * 0.5f - radius),
            glm::vec2(size.x * 0.5f - radius, radius - size.y * 0.5f),
            glm::vec2(radius - size.x * 0.5f, radius - size.y * 0.5f),
        };

        const float step = glm::radians(90.0f / segments);
        const size_t vertOffset = vertices.size();
        vertices.resize(vertOffset + (segments - 1) * 4 + points.size());

        for (size_t i = 0; i < points.size(); i++) {
            vertices[vertOffset + i].Position = { points[i].x, points[i].y, 0.f };
            vertices[vertOffset + i].TextureCoords = 0.5f + points[i] / size;
        }

        for (size_t k = 0; k < 4; k++) {
            if (segments == 1) {
                indices.emplace_back(vertOffset + k + 8);
                indices.emplace_back(vertOffset + k * 2);
                indices.emplace_back(vertOffset + k * 2 + 1);

                continue;
            }

            float angle = glm::radians(90.f - k * 90.f);
            const glm::vec2& center = points[8 + k];
            
            for (int i = 1; i < segments; i++) {
                angle += step;
                glm::vec2 offset(glm::cos(angle) * radius, glm::sin(angle) * radius);

                vertices[vertOffset + points.size() + (segments - 1) * k + i - 1].Position = { center.x + offset.x, center.y + offset.y, 0.f };
                vertices[vertOffset + points.size() + (segments - 1) * k + i - 1].TextureCoords = vertices[vertOffset + k + 8].TextureCoords + offset / size;
            }

            indices.emplace_back(vertOffset + k + 8);
            indices.emplace_back(vertOffset + k * 2 + 1);
            indices.emplace_back(vertOffset + points.size() + (segments - 1) * k);

            for (uint32_t i = 1; i < segments - 1; i++) {
                indices.emplace_back(vertOffset + k + 8);
                indices.emplace_back(vertOffset + points.size() + (segments - 1) * k + i - 1);
                indices.emplace_back(vertOffset + points.size() + (segments - 1) * k + i);
            }
            
            indices.emplace_back(vertOffset + k + 8);
            indices.emplace_back(vertOffset + points.size() + (segments - 1) * (k + 1) - 1);
            indices.emplace_back(vertOffset + k * 2);
        }

        for (size_t i = 0; i < 3; i += 2) {
            indices.emplace_back(vertOffset + 1 + i * 2);
            indices.emplace_back(vertOffset + 2 + i * 2);
            indices.emplace_back(vertOffset + 8 + i);

            indices.emplace_back(vertOffset + 2 + i * 2);
            indices.emplace_back(vertOffset + 8 + i);

            indices.emplace_back(vertOffset + 9 + i);
        }

        indices.emplace_back(vertOffset + 3);
        indices.emplace_back(vertOffset);
        indices.emplace_back(vertOffset + 4);
        indices.emplace_back(vertOffset);
        indices.emplace_back(vertOffset + 4);
        indices.emplace_back(vertOffset + 7);

        return ShapeError::None;
    }
    
    Shapes::ShapeError Shapes::RoundedQuadEdge(const glm::vec2& size, float thickness, float roundness, uint32_t segments, std::vector<Shapes::Vertex>& vertices, std::vector<uint32_t>& indices) {
        //Attribution: github.com/raysan5/raylib/src/rshapes.c
        if (roundness * segments <= 0.f) {
            return QuadEdge(size, thickness, vertices, indices);
        }
        if (vertices.capacity() - vertices.size() < (segments - 1) * 2 * 4 + 16) {
            return ShapeError::VertexOverflow;
        }
        if (indices.capacity() - indices.size() < (segments + 1) * 6 * 4) {
            return ShapeError::IndexOverflow;
        }
        if (size.x * size.y == 0.0f) {
            RC_WARN("Shapes::RoundedQuadEdge must have a non 0 size");
            return ShapeError::InvalidParameters;
        }

        roundness = std::min(roundness, 1.f) * 0.5f;
        const float radius = std::max(abs(size.x * roundness), abs(size.y * roundness));
        if (radius == 0.f) {
            return ShapeError::InvalidParameters;
        }

        const std::vector<glm::vec2> points = {
            glm::vec2(-size.x * 0.5f, size.y * 0.5f - radius),
            glm::vec2(radius - size.x * 0.5f, size.y * 0.5f),
            glm::vec2(size.x * 0.5f - radius, size.y * 0.5f),
            glm::vec2(size.x * 0.5f, size.y * 0.5f - radius),
            glm::vec2(size.x * 0.5f, radius - size.y * 0.5f),
            glm::vec2(size.x * 0.5f - radius, -size.y * 0.5f),
            glm::vec2(radius - size.x * 0.5f, -size.y * 0.5f),
            glm::vec2(-size.x * 0.5f, radius - size.y * 0.5f),

            glm::vec2(-size.x * 0.5f + thickness, size.y * 0.5f - radius),
            glm::vec2(radius - size.x * 0.5f, size.y * 0.5f - thickness),
            glm::vec2(size.x * 0.5f - radius, size.y * 0.5f - thickness),
            glm::vec2(size.x * 0.5f - thickness, size.y * 0.5f - radius),
            glm::vec2(size.x * 0.5f - thickness, radius - size.y * 0.5f),
            glm::vec2(size.x * 0.5f - radius, -size.y * 0.5f + thickness),
            glm::vec2(radius - size.x * 0.5f, -size.y * 0.5f + thickness),
            glm::vec2(-size.x * 0.5f + thickness, radius - size.y * 0.5f),
        };
        
        const std::vector<glm::vec2> centers = {
            glm::vec2(radius - size.x * 0.5f, size.y * 0.5f - radius),
            glm::vec2(size.x * 0.5f - radius, size.y * 0.5f - radius),
            glm::vec2(size.x * 0.5f - radius, radius - size.y * 0.5f),
            glm::vec2(radius - size.x * 0.5f, radius - size.y * 0.5f),
        };

        const float step = glm::radians(90.0f / segments);
        const size_t vertOffset = vertices.size();
        vertices.resize(vertOffset + points.size() + (segments - 1) * 8);

        for (size_t i = 0; i < points.size(); i++) {
            vertices[vertOffset + i].Position = { points[i].x, points[i].y, 0.f };
            vertices[vertOffset + i].TextureCoords = 0.5f + points[i] / size;
        }

        for (size_t k = 0; k < 4; k++) {
            indices.emplace_back(vertOffset + k * 2);
            indices.emplace_back(vertOffset + k * 2 + 8);
            indices.emplace_back(vertOffset + (k * 2 - 1) % 8);

            indices.emplace_back(vertOffset + k * 2 + 8);
            indices.emplace_back(vertOffset + (k * 2 - 1) % 8);
            indices.emplace_back(vertOffset + (k * 2 - 1) % 8 + 8);

            if (segments == 1) {
                indices.emplace_back(vertOffset + k * 2);
                indices.emplace_back(vertOffset + k * 2 + 1);
                indices.emplace_back(vertOffset + k * 2 + 8);
                
                indices.emplace_back(vertOffset + k * 2 + 1);
                indices.emplace_back(vertOffset + k * 2 + 8);
                indices.emplace_back(vertOffset + k * 2 + 9);
                continue;
            }

            float angle = glm::radians(90.f - k * 90.f);
            const glm::vec2& center = centers[k];
            const glm::vec2& textureCenter = 0.5f + centers[k] / size;
            
            for (int i = 1; i < segments; i++) {
                angle += step;
                glm::vec2 offset(glm::cos(angle), glm::sin(angle));

                vertices[vertOffset + points.size() + (segments - 1) * k * 2 + 2 * i - 2].Position = { center.x + offset.x * radius, center.y + offset.y * radius, 0.f };
                vertices[vertOffset + points.size() + (segments - 1) * k * 2 + 2 * i - 2].TextureCoords = textureCenter + offset * radius / size;
            
                vertices[vertOffset + points.size() + (segments - 1) * k * 2 + 2 * i - 1].Position = { center.x + offset.x * (radius - thickness), center.y + offset.y * (radius - thickness), 0.f };
                vertices[vertOffset + points.size() + (segments - 1) * k * 2 + 2 * i - 1].TextureCoords = textureCenter + offset * (radius - thickness) / size;
            }
            
            indices.emplace_back(vertOffset + k * 2 + 1);
            indices.emplace_back(vertOffset + k * 2 + 8 + 1);
            indices.emplace_back(vertOffset + points.size() + (segments - 1) * k * 2);
            
            indices.emplace_back(vertOffset + k * 2 + 8 + 1);
            indices.emplace_back(vertOffset + points.size() + (segments - 1) * k * 2);
            indices.emplace_back(vertOffset + points.size() + (segments - 1) * k * 2 + 1);
            
            for (uint32_t i = 1; i < segments - 1; i++) {
                indices.emplace_back(vertOffset + points.size() + (segments - 1) * k * 2 + i * 2 - 2);
                indices.emplace_back(vertOffset + points.size() + (segments - 1) * k * 2 + i * 2 - 1);
                indices.emplace_back(vertOffset + points.size() + (segments - 1) * k * 2 + i * 2);

                indices.emplace_back(vertOffset + points.size() + (segments - 1) * k * 2 + i * 2 - 1);
                indices.emplace_back(vertOffset + points.size() + (segments - 1) * k * 2 + i * 2);
                indices.emplace_back(vertOffset + points.size() + (segments - 1) * k * 2 + i * 2 + 1);
            }

            indices.emplace_back(vertOffset + k * 2);
            indices.emplace_back(vertOffset + k * 2 + 8);
            indices.emplace_back(vertOffset + points.size() + (segments - 1) * (k + 1) * 2 - 2);
            
            indices.emplace_back(vertOffset + k * 2 + 8);
            indices.emplace_back(vertOffset + points.size() + (segments - 1) * (k + 1) * 2 - 2);
            indices.emplace_back(vertOffset + points.size() + (segments - 1) * (k + 1) * 2 - 1);
        }

        return ShapeError::None;
    }

    std::vector<glm::vec2> Shapes::CirclePositions(uint32_t segments, float radius) {
        std::vector<glm::vec2> result;

        if (segments < 3) {
            RC_WARN("Shapes::Circle must have more than 2 segments");
            return result;
        }
        if (radius == 0.0f) {
            RC_WARN("Shapes::Circle must have a non 0 radius");
            return result;
        }
        float step = glm::radians(360.0f / segments);

        float angle = glm::radians(90.f);
        
        result.emplace_back(0.f);
        result.emplace_back(glm::cos(angle - step) * radius, glm::sin(angle - step) * radius);
        result.emplace_back(glm::cos(angle) * radius, glm::sin(angle) * radius);

        for (uint32_t i = 1; i < segments; i++) {
            angle += step;
            
            result.emplace_back(0.f);
            result.push_back(result[(i -1) * 3 + 2]);
            result.emplace_back(glm::cos(angle) * radius, glm::sin(angle) * radius);
        }

        return result;
    }

    Shapes::ShapeError Shapes::Circle(uint32_t segments, float radius, std::vector<Shapes::Vertex>& vertices, std::vector<uint32_t>& indices) {
        if (vertices.capacity() - vertices.size() < segments + 1) {
            return ShapeError::VertexOverflow;
        }
        if (indices.capacity() - indices.size() < segments * 3) {
            return ShapeError::IndexOverflow;
        }
        if (segments < 3) {
            RC_WARN("Shapes::Circle must have more than 2 segments");
            return ShapeError::InvalidParameters;
        }
        if (radius == 0.0f) {
            RC_WARN("Shapes::Circle must have a non 0 radius");
            return ShapeError::InvalidParameters;
        }
        
        float step = glm::radians(360.0f / segments);
        
        size_t vertOffset = vertices.size();
        vertices.resize(vertOffset + segments + 1);
        vertices[vertOffset].Position = { 0.f, 0.f, 0.f };
        vertices[vertOffset].TextureCoords = { 0.5f, 0.5f };

        float angle = glm::radians(90.f);
        for (uint32_t i = 0; i < segments; i++) {
            float x = glm::cos(angle);
            float y = glm::sin(angle);
            vertices[vertOffset + i + 1].Position = { x * radius, y * radius, 0.f };
            vertices[vertOffset + i + 1].TextureCoords = { 0.5f - x * 0.5f, 0.5f + y * 0.5f};
            angle += step;
        }

        size_t indOffset = indices.size();

        for (uint32_t i = 2; i <= segments; i++) {
            indices.emplace_back(vertOffset);
            indices.emplace_back(vertOffset + i - 1);
            indices.emplace_back(vertOffset + i);
        }

        indices.emplace_back(vertOffset);
        indices.emplace_back(vertOffset + segments);
        indices.emplace_back(vertOffset + 1);

        return ShapeError::None;
    }
}