#include "Renderer2D.h"

#include "RenderAPI.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "ElementBuffer.h"
#include "Shader.h"
#include "Texture.h"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>

#include <memory>

struct Renderer2DDAta {
	std::unique_ptr<Core::VertexArray> LineVertexArray;
	std::unique_ptr<Core::VertexBuffer> LineVertexBuffer;

	std::unique_ptr<Core::VertexArray> QuadVertexArray;
	std::unique_ptr<Core::VertexBuffer> QuadVertexBuffer;
	std::unique_ptr<Core::ElementBuffer> QuadElementBuffer;

	std::unique_ptr<Core::Shader> FlatShader;
	std::unique_ptr<Core::Shader> TextureShader;
	std::unique_ptr<Core::Shader> TextShader;

	std::unique_ptr<Core::Texture2D> TextureAtlas;

	std::shared_ptr<Core::Font> Font;

	glm::mat4 ViewProjection = glm::mat4(1.0f);

	uint32_t atlasWidth = 0;
	uint32_t atlasHeight = 0;
};

static Renderer2DDAta s_Data;

namespace Core {
	void Renderer2D::Init(){
		const float lineVertices[] = {
			0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
		};

		s_Data.LineVertexArray = std::make_unique<VertexArray>();;
		s_Data.LineVertexBuffer = std::make_unique<VertexBuffer>(lineVertices, sizeof(lineVertices));
		Core::VertexBufferLayout lineLayout;

		lineLayout.Push<float>(3);
		s_Data.LineVertexArray->AddBuffer(*s_Data.LineVertexBuffer, lineLayout);
		
		const float quadVertices[]{
			 0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
			 0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
			-0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
			-0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
		};

		const uint32_t quadIndices[]{
			0, 1, 2,
			1, 2, 3,
		};

		s_Data.QuadVertexArray = std::make_unique<VertexArray>();
		s_Data.QuadVertexBuffer = std::make_unique<VertexBuffer>(quadVertices, sizeof(quadVertices));
		VertexBufferLayout quadLayout;

		quadLayout.Push<float>(3);
		quadLayout.Push<float>(2);
		s_Data.QuadVertexArray->AddBuffer(*s_Data.QuadVertexBuffer, quadLayout);

		s_Data.QuadElementBuffer = std::make_unique<ElementBuffer>(quadIndices, 6);

		s_Data.TextureShader = std::make_unique<Shader>("2DTextureShader.glsl");
		s_Data.TextureShader->Bind();
		
		s_Data.TextureAtlas = std::make_unique<Texture2D>(GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
		{
			unsigned char missingTextureData[] = {
				0, 0, 0, 255,
				255, 0, 255, 255,
				255, 0, 255, 255,
				0, 0, 0, 255,
			};
			s_Data.TextureAtlas->BindData(missingTextureData, 2, 2, 4);
		}
		s_Data.TextureShader->setInt("tex", 0);

		s_Data.atlasWidth = s_Data.atlasHeight = 1;
		s_Data.TextureShader->setVec2("atlasSize", glm::vec2(s_Data.atlasWidth, s_Data.atlasHeight));

		s_Data.FlatShader = std::make_unique<Shader>("2DFlatShader.glsl");

		s_Data.TextShader = std::make_unique<Shader>("TextShader.glsl");
		s_Data.Font = std::make_shared<Font>();

		s_Data.ViewProjection = glm::mat4(1.0f);
	}

	void Renderer2D::BeginScene(const Camera& camera) {
		s_Data.ViewProjection = camera.GetViewMatrix();

		s_Data.FlatShader->Bind();
		s_Data.FlatShader->setMat4("viewProjection", s_Data.ViewProjection);

		s_Data.TextureShader->Bind();
		s_Data.TextureShader->setMat4("viewProjection", s_Data.ViewProjection);

		s_Data.TextShader->Bind();
		s_Data.TextShader->setMat4("viewProjection", s_Data.ViewProjection);

		s_Data.TextureAtlas->Activate(0);
	}

	void Renderer2D::BeginScene(const glm::mat4& transform) {
		s_Data.ViewProjection = transform;

		s_Data.FlatShader->Bind();
		s_Data.FlatShader->setMat4("viewProjection", s_Data.ViewProjection);

		s_Data.TextureShader->Bind();
		s_Data.TextureShader->setMat4("viewProjection", s_Data.ViewProjection);

		s_Data.TextShader->Bind();
		s_Data.TextShader->setMat4("viewProjection", s_Data.ViewProjection);

		s_Data.TextureAtlas->Activate(0);
	}

	void Renderer2D::DrawTextureQuad(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& colour, const glm::vec2& textureOffset, const glm::vec2& textureScale, uint32_t atlasIndex, float textureRotate){
		RenderAPI::SetDepthBuffer(false);

		s_Data.TextureShader->Bind();
		s_Data.TextureShader->setVec3("colour", colour);
		s_Data.TextureShader->setVec2("atlasOffset", glm::vec2(atlasIndex % s_Data.atlasWidth, atlasIndex / s_Data.atlasWidth));

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::scale(transform, scale);
		s_Data.TextureShader->setMat4("transform", transform);

		glm::mat3 texTransform = glm::translate(glm::mat3(1.0f), textureOffset);
		texTransform = glm::rotate(texTransform, glm::radians(textureRotate));
		texTransform = glm::scale(texTransform, textureScale);
		s_Data.TextureShader->setMat3("texTransform", texTransform);

		RenderAPI::DrawIndexed(*s_Data.QuadVertexArray, 6);
	}

	void Renderer2D::DrawFlatQuad(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& colour) {
		RenderAPI::SetDepthBuffer(false);
		
		s_Data.FlatShader->Bind();
		s_Data.FlatShader->setVec3("colour", colour);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::scale(transform, scale);

		s_Data.FlatShader->setMat4("transform", transform);

		RenderAPI::DrawIndexed(*s_Data.QuadVertexArray, 6);
	}

	void Renderer2D::DrawRotatedFlatQuad(const glm::vec3& position, float rotation, const glm::vec3& rotationAxis, const glm::vec3& scale, const glm::vec3& colour) {
		RenderAPI::SetDepthBuffer(false);
		s_Data.FlatShader->Bind();
		s_Data.FlatShader->setVec3("colour", colour);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::rotate(transform, glm::radians(rotation), rotationAxis);
		transform = glm::scale(transform, scale);

		s_Data.FlatShader->setMat4("transform", transform);

		RenderAPI::DrawIndexed(*s_Data.QuadVertexArray, 6);
	}

	void Renderer2D::DrawRotatedFlatTriangle(const glm::vec3& position, float rotation, const glm::vec3& rotationAxis, const glm::vec3& scale, const glm::vec3& colour) {
		RenderAPI::SetDepthBuffer(false);
		s_Data.FlatShader->Bind();
		s_Data.FlatShader->setVec3("colour", colour);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::rotate(transform, glm::radians(rotation), rotationAxis);
		transform = glm::scale(transform, scale);

		s_Data.FlatShader->setMat4("transform", transform);
		
		RenderAPI::DrawIndexed(*s_Data.QuadVertexArray, 3);
	}

	void Renderer2D::DrawLine(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& colour) {
		RenderAPI::SetDepthBuffer(false);
		
		s_Data.FlatShader->Bind();
		s_Data.FlatShader->setVec3("colour", colour);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::scale(transform, scale);
		s_Data.FlatShader->setMat4("transform", transform);

		s_Data.LineVertexArray->Bind();
		RenderAPI::DrawLines(*s_Data.LineVertexArray, 2);
	}

	template <typename T>
	void Renderer2D::DrawString(const T& text, float x, float y, float scale, const glm::vec3& colour) {
		RenderAPI::SetDepthBuffer(false);

		s_Data.TextShader->Bind();
		s_Data.TextShader->setVec3("colour", colour);
		s_Data.Font->ActivateAtlas();
		
		glm::vec3 position(0.0f);
		glm::vec3 size(1.0f);
		float startX = x;
		for (auto c = text.begin(); c != text.end(); c++) {
			if (*c == '\r') {
				continue;
			}
			if (*c == '\n') {
				x = startX;
				y -= s_Data.Font->GetGlyphInfo(' ').Size.y * scale;
				continue;
			}

			GlyphInfo glyph = s_Data.Font->GetGlyphInfo(*c);
			
			if (glyph.Advance == 0) {
				glyph = s_Data.Font->GetGlyphInfo('?');
			}
			if (glyph.Size.x * glyph.Size.y == 0) {
				x += glyph.Advance * scale;
				continue;
			}

			size.x = glyph.Size.x * scale;
			size.y = -glyph.Size.y * scale;

			position.x = x + glyph.Bearing.x * scale + size.x * 0.5f;
			position.y = y - (glyph.Size.y - glyph.Bearing.y) * scale - size.y * 0.5f;

			glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
			transform = glm::scale(transform, size);
			s_Data.TextShader->setMat4("transform", transform);

			glm::mat3 texTransform = glm::translate(glm::mat3(1.0f), glyph.TexPosition);
			texTransform = glm::scale(texTransform, glyph.TexScale);
			s_Data.TextShader->setMat3("texTransform", texTransform);

			RenderAPI::DrawIndexed(*s_Data.QuadVertexArray, 6);
			x += glyph.Advance * scale;
		}
	}

	void Renderer2D::SetTextureAtlas(const char* fileName, uint32_t width, uint32_t height) {
		s_Data.TextureShader->Bind();
		s_Data.TextureAtlas->BindImage(fileName);

		s_Data.atlasWidth = width;
		s_Data.atlasHeight = height;
		s_Data.TextureShader->setVec2("atlasSize", glm::vec2(s_Data.atlasWidth, s_Data.atlasHeight));
	}

	void Renderer2D::SetFont(std::shared_ptr<Font> font) {
		s_Data.Font = font;
	}

	template void Renderer2D::DrawString<std::string>(const std::string&, float, float, float, const glm::vec3&);
	template void Renderer2D::DrawString<std::wstring>(const std::wstring&, float, float, float, const glm::vec3&);
}

