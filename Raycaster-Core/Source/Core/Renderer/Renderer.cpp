#include "Renderer.h"
#include "RenderAPI.h"

static glm::mat4 s_ViewProjectionMatrix = glm::mat4(1.0f);

namespace Core {
	void Renderer::BeginScene(Camera& camera) {
		s_ViewProjectionMatrix = camera.GetViewMatrix();
	}

	void Renderer::BeginScene(glm::mat4& transform) {
		RenderAPI::SetDepthBuffer(true);
		s_ViewProjectionMatrix = transform;
	}

	void Renderer::DrawModel(const Model& model, const glm::mat4& transform, const glm::vec3& tint) {
		glm::mat4 modelTransform = transform * model.Transform;
		
		for (auto& indexedMesh : model.Meshes) {
			DrawMesh(*indexedMesh.Mesh, *model.Materials[indexedMesh.MaterialIndex], modelTransform, tint);
		}
	}
	
	void Renderer::DrawMesh(const Mesh& mesh, const Material& material, const glm::mat4& transform, const glm::vec3& tint) {
		material.Shader->Bind();
		material.Shader->setMat4("ViewProjection", s_ViewProjectionMatrix);
		material.Shader->setMat4("ModelTransform", transform);
		material.Shader->setVec3("ModelTint", tint);
			
		for (auto& materialMap : material.MaterialMaps) {
			materialMap.Texture->Activate(materialMap.TextureIndex);

			if (!materialMap.ColourUniform.empty()) {
				material.Shader->setVec3(materialMap.ColourUniform.c_str(), materialMap.Colour);
			}
		}

		if (mesh.EBO != nullptr) {
			RenderAPI::DrawIndexed(*mesh.VAO, mesh.EBO->GetCount());
		} else {
			RenderAPI::DrawVertices(*mesh.VAO, mesh.VertexCount);
		}
	}
}