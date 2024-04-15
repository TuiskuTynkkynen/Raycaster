#pragma once

#include "Camera.h"

namespace Core {
	class Renderer2D //TODO implement batch rendering
	{
	public: 
		static void Init();

		static void BeginScene(RaycasterCamera& camera);
		static void BeginScene(glm::mat4& transform);
		static void Clear(glm::vec3& colour);

		static void SetViewPort(uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height);

		static void DrawTextureQuad(glm::vec3& position, glm::vec3& scale, glm::vec3& colour, glm::vec2& textureOffset, glm::vec2& textureScale, uint32_t atlasIndex = 0, float textureRotate = 0.0f);
		static void DrawFlatQuad(glm::vec3& position, glm::vec3& scale, glm::vec3& colour);
		static void DrawRotatedFlatQuad(glm::vec3& position, float rotation, glm::vec3& rotationAxis, glm::vec3& scale, glm::vec3& colour);
		static void DrawRotatedFlatTriangle(glm::vec3& position, float rotation, glm::vec3& rotationAxis, glm::vec3& scale, glm::vec3& colour);
		static void DrawLine(glm::vec3& position, glm::vec3& scale, glm::vec3& colour);

		static void SetLineWidth(uint32_t width);
	};
}