#pragma once

#include "Camera.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "ElementBuffer.h"
#include "Shader.h"
#include "Texture.h"

#include <memory>

namespace Core {
	class Renderer2D //TODO implement batch rendering
	{
	private: 
		std::unique_ptr<VertexArray> m_QuadVertexArray;
		std::unique_ptr<VertexBuffer> m_QuadVertexBuffer;
		std::unique_ptr<ElementBuffer> m_QuadElementBuffer;

		std::unique_ptr<VertexArray> m_LineVertexArray;
		std::unique_ptr<VertexBuffer> m_LineVertexBuffer;

		std::unique_ptr<Shader> m_FlatShader;

		std::unique_ptr<Shader> m_TextureShader;
		std::unique_ptr<Texture2D> m_TextureAtlas;

		glm::mat4 m_ViewProjection;
	public: 
		Renderer2D();

		void BeginScene(RaycasterCamera& camera);
		void BeginScene(glm::mat4& transform);
		void Clear(glm::vec3& colour);

		void SetViewPort(uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height);

		void DrawTextureQuad(glm::vec3& position, glm::vec3& scale, glm::vec3& colour, glm::vec2& textureOffset, glm::vec2& textureScale);
		void DrawFlatQuad(glm::vec3& position, glm::vec3& scale, glm::vec3& colour);
		void DrawRotatedFlatQuad(glm::vec3& position, float rotation, glm::vec3& rotationAxis, glm::vec3& scale, glm::vec3& colour);
		void DrawLine(glm::vec3& position, glm::vec3& scale, glm::vec3& colour);

		void SetLineWidth(uint32_t width);
	};
}