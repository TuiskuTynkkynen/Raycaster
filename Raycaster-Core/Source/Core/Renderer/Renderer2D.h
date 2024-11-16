#pragma once

#include "Camera.h"
#include "Core/Font/Font.h"

#include <string>

namespace Core::Renderer2D {
        void Init();
         
        void BeginScene(const Camera& camera);
        void BeginScene(const glm::mat4& transform);
        void EndScene();
        void Flush();
         
        void DrawQuad(uint32_t textureIndex, const glm::vec4& colour, const glm::mat4& transform, const glm::mat3& textureTransform = glm::mat3(1.0f), const glm::vec2& atlasIndex = glm::vec2(0.0f));
        void DrawTextureQuad(const glm::vec3& position, const glm::vec3& scale, const glm::vec4& colour, const glm::vec2& textureOffset, const glm::vec2& textureScale, uint32_t atlasIndex = 0, float textureRotate = 0.0f);
        void DrawFlatQuad(const glm::vec3& position, const glm::vec3& scale, const glm::vec4& colour);
        void DrawRotatedFlatQuad(const glm::vec3& position, float rotation, const  glm::vec3& rotationAxis, const  glm::vec3& scale, const glm::vec4& colour);
        void DrawRotatedFlatTriangle(const glm::vec3& position, float rotation, const glm::vec3& rotationAxis, const glm::vec3& scale, const glm::vec4& colour);
        void DrawLine(const glm::vec3& position, const glm::vec3& scale, const glm::vec4& colour);
        
        void DrawTriangle(const glm::vec3& angles, uint32_t textureIndex, const glm::vec4& colour, const glm::mat4& transform, const glm::mat3& textureTransform = glm::mat3(1.0f), const glm::vec2& atlasIndex = glm::vec2(0.0f));
        void DrawTextureTriangle(const glm::vec3& angles, const glm::vec3& position, const glm::vec3& scale, const glm::vec4& colour, const glm::vec2& textureOffset, const glm::vec2& textureScale, uint32_t atlasIndex = 0, float textureRotate = 0.0f);
        void DrawFlatTriangle(const glm::vec3& angles, const glm::vec3& position, const glm::vec3& scale, const glm::vec4& colour);
        void DrawRotatedFlatTriangle(const glm::vec3& angles, const glm::vec3& position, float rotation, const  glm::vec3& rotationAxis, const  glm::vec3& scale, const glm::vec4& colour);

        template <typename T>
        void DrawString(const T& text, float x, float y, float scale, const glm::vec4& colour, bool flipVertivcal = false);

        void SetTextureAtlas(const char* fileName, uint32_t width, uint32_t height);
        void SetFont(std::shared_ptr<Font> font);

        //Using Core::Shapes functions
        void DrawShapeQuad(uint32_t textureIndex, const glm::vec4 & colour, const glm::mat4 & transform, const glm::mat3 & textureTransform = glm::mat3(1.0f), const glm::vec2 & atlasIndex = glm::vec2(0.0f));
        void DrawTextureShapeQuad(const glm::vec3 & position, const glm::vec3 & scale, const glm::vec4 & colour, const glm::vec2 & textureOffset, const glm::vec2 & textureScale, uint32_t atlasIndex = 0, float textureRotate = 0.0f);
        void DrawFlatShapeQuad(const glm::vec3 & position, const glm::vec3 & scale, const glm::vec4 & colour);
        void DrawRotatedFlatShapeQuad(const glm::vec3 & position, float rotation, const  glm::vec3 & rotationAxis, const  glm::vec3 & scale, const glm::vec4 & colour);

        void DrawQuadEdge(const glm::vec2& size, float thickness, uint32_t textureIndex, const glm::vec4& colour, const glm::mat4& transform, const glm::mat3& textureTransform = glm::mat3(1.0f), const glm::vec2& atlasIndex = glm::vec2(0.0f));
        void DrawTextureQuadEdge(const glm::vec2& size, float thickness, const glm::vec3& position, const glm::vec3& scale, const glm::vec4& colour, const glm::vec2& textureOffset, const glm::vec2& textureScale, uint32_t atlasIndex = 0, float textureRotate = 0.0f);
        void DrawFlatQuadEdge(const glm::vec2& size, float thickness, const glm::vec3& position, const glm::vec3& scale, const glm::vec4& colour);
        void DrawRotatedFlatQuadEdge(const glm::vec2& size, float thickness, const glm::vec3 & position, float rotation, const  glm::vec3 & rotationAxis, const  glm::vec3 & scale, const glm::vec4 & colour);

        void DrawRoundedQuad(const glm::vec2 & size, float roundness, uint32_t segments, uint32_t textureIndex, const glm::vec4 & colour, const glm::mat4 & transform, const glm::mat3 & textureTransform = glm::mat3(1.0f), const glm::vec2 & atlasIndex = glm::vec2(0.0f));
        void DrawTextureRoundedQuad(const glm::vec2 & size, float roundness, uint32_t segments, const glm::vec3 & position, const glm::vec3 & scale, const glm::vec4 & colour, const glm::vec2 & textureOffset, const glm::vec2 & textureScale, uint32_t atlasIndex = 0, float textureRotate = 0.0f);
        void DrawFlatRoundedQuad(const glm::vec2 & size, float roundness, uint32_t segments, const glm::vec3 & position, const glm::vec3 & scale, const glm::vec4 & colour);
        void DrawRotatedFlatRoundedQuad(const glm::vec2 & size, float roundness, uint32_t segments, const glm::vec3 & position, float rotation, const  glm::vec3 & rotationAxis, const  glm::vec3 & scale, const glm::vec4 & colour);

        void DrawRoundedQuadEdge(const glm::vec2& size, float thickness, float roundness, uint32_t segments, uint32_t textureIndex, const glm::vec4& colour, const glm::mat4& transform, const glm::mat3& textureTransform = glm::mat3(1.0f), const glm::vec2& atlasIndex = glm::vec2(0.0f));
        void DrawTextureRoundedQuadEdge(const glm::vec2& size, float thickness, float roundness, uint32_t segments, const glm::vec3& position, const glm::vec3& scale, const glm::vec4& colour, const glm::vec2& textureOffset, const glm::vec2& textureScale, uint32_t atlasIndex = 0, float textureRotate = 0.0f);
        void DrawFlatRoundedQuadEdge(const glm::vec2& size, float thickness, float roundness, uint32_t segments, const glm::vec3& position, const glm::vec3& scale, const glm::vec4& colour);
        void DrawRotatedFlatRoundedQuadEdge(const glm::vec2& size, float thickness, float roundness, uint32_t segments, const glm::vec3 & position, float rotation, const  glm::vec3 & rotationAxis, const  glm::vec3 & scale, const glm::vec4 & colour);

        void DrawCircle(uint32_t segments, uint32_t textureIndex, const glm::vec4 & colour, const glm::mat4 & transform, const glm::mat3 & textureTransform = glm::mat3(1.0f), const glm::vec2 & atlasIndex = glm::vec2(0.0f));
        void DrawTextureCircle(uint32_t segments, const glm::vec3 & position, const glm::vec3 & scale, const glm::vec4 & colour, const glm::vec2 & textureOffset, const glm::vec2 & textureScale, uint32_t atlasIndex = 0, float textureRotate = 0.0f);
        void DrawFlatCircle(uint32_t segments, const glm::vec3 & position, const glm::vec3 & scale, const glm::vec4 & colour);
        void DrawRotatedFlatCircle(uint32_t segments, const glm::vec3 & position, float rotation, const  glm::vec3 & rotationAxis, const  glm::vec3 & scale, const glm::vec4 & colour);

        template <typename T>
        void DrawShapeString(const T & text, float x, float y, float scale, const glm::vec4 & colour, bool flipVertivcal = false);

}