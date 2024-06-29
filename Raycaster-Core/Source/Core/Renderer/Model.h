#pragma once

#include "VertexArray.h"
#include "ElementBuffer.h"
#include "Texture.h"
#include "Shader.h"

#include <glm/glm.hpp>

#include <memory>
#include <variant>
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
        struct Parameter {
            std::variant<float, glm::vec2, glm::vec3, glm::vec4> Value;
            std::string Uniform;

            Parameter(std::variant<float, glm::vec2, glm::vec3, glm::vec4> value, const std::string& uniformName)
                : Value(value), Uniform(uniformName)
            {}
        };

        std::shared_ptr<Shader> Shader;
        std::vector<MaterialMap> MaterialMaps;
        std::vector<Parameter> Parameters;
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