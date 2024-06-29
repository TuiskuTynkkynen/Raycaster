#pragma once

#include "Camera.h"
#include "Model.h"

namespace Core::Renderer {
    void BeginScene(const Camera& camera);
    void BeginScene(const glm::mat4& transform);

    void DrawModel(const Model& model, const glm::mat4& transform = glm::mat4(1.0f), const glm::vec3& tint = glm::vec4(1.0f));
    void DrawMesh(const Mesh& mesh, const Material& material, const glm::mat4& transform = glm::mat4(1.0f), const glm::vec3& tint = glm::vec4(1.0f));
}