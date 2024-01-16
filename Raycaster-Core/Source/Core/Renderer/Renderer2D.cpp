#include "Renderer2D.h"

#include "RenderAPI.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "ElementBuffer.h"
#include "Shader.h"
#include "Texture.h"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

struct Renderer2DDAta {
	std::unique_ptr<Core::VertexArray> LineVertexArray;
	std::unique_ptr<Core::VertexBuffer> LineVertexBuffer;

	std::unique_ptr<Core::VertexArray> QuadVertexArray;
	std::unique_ptr<Core::VertexBuffer> QuadVertexBuffer;
	std::unique_ptr<Core::ElementBuffer> QuadElementBuffer;

	std::unique_ptr<Core::Shader> FlatShader;
	std::unique_ptr<Core::Shader> TextureShader;

	std::unique_ptr<Core::Texture2D> TextureAtlas;

	glm::mat4 ViewProjection;
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
		s_Data.TextureAtlas->BindImage("crate.png");

		s_Data.TextureAtlas->Activate(0);
		s_Data.TextureShader->setInt("tex", 0);

		s_Data.FlatShader = std::make_unique<Shader>("2DFlatShader.glsl");

		s_Data.ViewProjection = glm::mat4(1.0f);
	}

	void Renderer2D::BeginScene(RaycasterCamera& camera) {
		s_Data.ViewProjection = camera.GetViewMatrix();
	}

	void Renderer2D::BeginScene(glm::mat4& transform) {
		s_Data.ViewProjection = transform;
	}

	void Renderer2D::Clear(glm::vec3& colour) {
		RenderAPI::SetClearColour(colour);
		RenderAPI::Clear();
	}

	void Renderer2D::SetViewPort(uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height) {
		RenderAPI::SetViewPort(offsetX, offsetY, width, height);
	}

	void Renderer2D::DrawTextureQuad(glm::vec3& position, glm::vec3& scale, glm::vec3& colour, glm::vec2& textureOffset, glm::vec2& textureScale){
		RenderAPI::SetDepthBuffer(false);

		s_Data.TextureShader->Bind();
		s_Data.TextureShader->setVec3("colour", colour);
		s_Data.TextureShader->setVec2("texTranslate", textureOffset);
		s_Data.TextureShader->setVec2("texScale", textureScale);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::scale(transform, scale);

		s_Data.TextureShader->setMat4("transform", s_Data.ViewProjection * transform);

		s_Data.QuadVertexArray->Bind();
		RenderAPI::DrawIndexed(*s_Data.QuadVertexArray, 6);
	}

	void Renderer2D::DrawFlatQuad(glm::vec3& position, glm::vec3& scale, glm::vec3& colour) {
		RenderAPI::SetDepthBuffer(false);
		
		s_Data.FlatShader->Bind();
		s_Data.FlatShader->setVec3("colour", colour);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::scale(transform, scale);

		s_Data.FlatShader->setMat4("transform", s_Data.ViewProjection * transform);

		s_Data.QuadVertexArray->Bind();
		RenderAPI::DrawIndexed(*s_Data.QuadVertexArray, 6);
	}

	void Renderer2D::DrawRotatedFlatQuad(glm::vec3& position, float rotation, glm::vec3& rotationAxis, glm::vec3& scale, glm::vec3& colour) {
		RenderAPI::SetDepthBuffer(false);
		s_Data.FlatShader->Bind();
		s_Data.FlatShader->setVec3("colour", colour);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::rotate(transform, glm::radians(rotation), rotationAxis);
		transform = glm::scale(transform, scale);

		s_Data.FlatShader->setMat4("transform", s_Data.ViewProjection * transform);

		s_Data.QuadVertexArray->Bind();
		RenderAPI::DrawIndexed(*s_Data.QuadVertexArray, 6);
	}

	void Renderer2D::DrawLine(glm::vec3& position, glm::vec3& scale, glm::vec3& colour) {
		RenderAPI::SetDepthBuffer(false);
		
		s_Data.FlatShader->Bind();
		s_Data.FlatShader->setVec3("colour", colour);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::scale(transform, scale);
		s_Data.FlatShader->setMat4("transform", s_Data.ViewProjection * transform);

		s_Data.LineVertexArray->Bind();
		RenderAPI::DrawLines(*s_Data.LineVertexArray, 2);
	}

	void Renderer2D::SetLineWidth(uint32_t width) {
		RenderAPI::SetLineWidth(width);
	}
}