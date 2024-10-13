#pragma once

#include "Camera.h"
#include "Core/Font/Font.h"

#include <string>

namespace Core {
	class Renderer2D //TODO implement batch rendering
	{
	public: 
		static void Init();

		static void BeginScene(const Camera& camera);
		static void BeginScene(const glm::mat4& transform);
		static void EndScene();
		static void Flush();

		static void DrawQuad(uint32_t textureIndex, const glm::vec4& colour, const glm::mat4& transform, const glm::mat3& textureTransform = glm::mat3(1.0f), const glm::vec2& atlasIndex = glm::vec2(0.0f));
		static void DrawTextureQuad(const glm::vec3& position, const glm::vec3& scale, const glm::vec4& colour, const glm::vec2& textureOffset, const glm::vec2& textureScale, uint32_t atlasIndex = 0, float textureRotate = 0.0f);
		static void DrawFlatQuad(const glm::vec3& position, const glm::vec3& scale, const glm::vec4& colour);
		static void DrawRotatedFlatQuad(const glm::vec3& position, float rotation, const  glm::vec3& rotationAxis, const  glm::vec3& scale, const glm::vec4& colour);
		static void DrawRotatedFlatTriangle(const glm::vec3& position, float rotation, const glm::vec3& rotationAxis, const glm::vec3& scale, const glm::vec4& colour);
		static void DrawLine(const glm::vec3& position, const glm::vec3& scale, const glm::vec4& colour);
		
		template <typename T>
		static void DrawString(const T& text, float x, float y, float scale, const glm::vec4& colour);

		static void SetTextureAtlas(const char* fileName, uint32_t width, uint32_t height);
		static void SetFont(std::shared_ptr<Font> font);
	};
}