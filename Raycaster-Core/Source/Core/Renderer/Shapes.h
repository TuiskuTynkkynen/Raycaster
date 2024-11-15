#pragma once

#include <glm/glm.hpp>

#include <vector>

namespace Core::Shapes {
    enum class ShapeError {
        None = 0,
        InvalidParameters,
        VertexOverflow,
        IndexOverflow,
    };

    struct Vertex {
        glm::vec3 Position{ 0.0f };
        glm::vec4 Colour{ 0.0f };
        glm::vec2 TextureCoords{ 0.0f };
        glm::vec2 AtlasOffset{ 0.0f };

        float TextureIndex = 0;
    };
    std::vector<glm::vec2> TrianglePositions(const glm::vec3& angles, float scale);
    ShapeError Triangle(const glm::vec3& angles, float scale, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

    std::vector<glm::vec2> QuadPositions(const glm::vec2& size);
    ShapeError Quad(const glm::vec2& size, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    
    std::vector<glm::vec2> QuadEdgePositions(const glm::vec2& size, float thickness);
    ShapeError QuadEdge(const glm::vec2& size, float thickness, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    
    std::vector<glm::vec2> RoundedQuadPositions(const glm::vec2& size, float roundness, uint32_t segments);
    ShapeError RoundedQuad(const glm::vec2& size, float roundness, uint32_t segments, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    
    ShapeError RoundedQuadEdge(const glm::vec2& size, float thickness, float roundness, uint32_t segments, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    
    std::vector<glm::vec2> CirclePositions(uint32_t segments, float radius);
    ShapeError Circle(uint32_t segments, float radius, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
}