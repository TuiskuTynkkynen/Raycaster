#pragma once

#include "Camera.h"
#include "Font.h"

#include <string>

namespace Core {
	class Renderer2D //TODO implement batch rendering
	{
	public: 
		static void Init();

		static void BeginScene(Camera& camera);
		static void BeginScene(glm::mat4& transform);

		static void DrawTextureQuad(glm::vec3& position, glm::vec3& scale, glm::vec3& colour, glm::vec2& textureOffset, glm::vec2& textureScale, uint32_t atlasIndex = 0, float textureRotate = 0.0f);
		static void DrawFlatQuad(glm::vec3& position, glm::vec3& scale, glm::vec3& colour);
		static void DrawRotatedFlatQuad(glm::vec3& position, float rotation, glm::vec3& rotationAxis, glm::vec3& scale, glm::vec3& colour);
		static void DrawRotatedFlatTriangle(glm::vec3& position, float rotation, glm::vec3& rotationAxis, glm::vec3& scale, glm::vec3& colour);
		static void DrawLine(glm::vec3& position, glm::vec3& scale, glm::vec3& colour);
		
		template <typename T>
		static void DrawString(const T& text, float x, float y, float scale, const glm::vec3& colour);

		static void SetTextureAtlas(const char* fileName, uint32_t width, uint32_t height);
		static void SetFont(std::shared_ptr<Font> font);
	};
}