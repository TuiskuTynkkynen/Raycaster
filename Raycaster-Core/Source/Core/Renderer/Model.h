#pragma once

#include "VertexArray.h"
#include "ElementBuffer.h"
#include "Texture.h"
#include "Shader.h"

#include <glm/glm.hpp>

#include <memory>
#include <string>

//TODO tie into ECS

namespace Core {
    struct Mesh {
        std::unique_ptr<VertexArray> VAO;
        std::unique_ptr<VertexBuffer> VBO;
        std::unique_ptr<ElementBuffer> EBO;
        uint32_t VertexCount{};
    };

    struct MaterialMap {
        std::shared_ptr<Texture2D> Texture;
        uint32_t TextureIndex{};

        glm::vec3 Colour;
        std::string ColourUniform;
    };

    struct Material {
        std::shared_ptr<Shader> Shader;
        std::vector<MaterialMap> MaterialMaps;
    };

    struct Model {
        struct  IndexedMesh {
            std::shared_ptr<Mesh> Mesh;
            uint32_t MaterialIndex;
        };

        std::vector<IndexedMesh> Meshes;
        std::vector<std::shared_ptr<Material>> Materials;
            
        glm::mat4 Transform;
    };
}