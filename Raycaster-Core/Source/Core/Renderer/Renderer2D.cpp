#include "Renderer2D.h"

#include "RenderAPI.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Core {
	static RenderAPI s_RenderAPI;

	Renderer2D::Renderer2D(){
		const float lineVertices[] = {
			0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
		};

		m_LineVertexArray = std::unique_ptr<VertexArray>(new VertexArray());
		m_LineVertexBuffer = std::unique_ptr<VertexBuffer>(new VertexBuffer(lineVertices, sizeof(lineVertices)));
		Core::VertexBufferLayout lineLayout;

		lineLayout.Push<float>(3);
		m_LineVertexArray->AddBuffer(*m_LineVertexBuffer, lineLayout);
		
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

		m_QuadVertexArray = std::unique_ptr<VertexArray>(new VertexArray());
		m_QuadVertexBuffer = std::unique_ptr<VertexBuffer>(new VertexBuffer(quadVertices, sizeof(quadVertices)));
		VertexBufferLayout quadLayout;

		quadLayout.Push<float>(3);
		quadLayout.Push<float>(2);
		m_QuadVertexArray->AddBuffer(*m_QuadVertexBuffer, quadLayout);

		m_QuadElementBuffer = std::unique_ptr<ElementBuffer>(new ElementBuffer(quadIndices, 6));

		m_TextureShader = std::unique_ptr<Shader>(new Shader("2DTextureShader.glsl"));
		m_TextureShader->Bind();
		
		m_TextureAtlas = std::unique_ptr<Texture2D>(new Texture2D(GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST));
		m_TextureAtlas->BindImage("crate.png");

		m_TextureAtlas->Activate(0);
		m_TextureShader->setInt("tex", 0);

		m_FlatShader = std::unique_ptr<Shader>(new Shader("2DFlatShader.glsl"));

		m_ViewProjection = glm::mat4(1.0f);
	}

	void Renderer2D::BeginScene(RaycasterCamera& camera) {
		m_ViewProjection = camera.GetViewMatrix();
	}

	void Renderer2D::BeginScene(glm::mat4& transform) {
		m_ViewProjection = transform;
	}

	void Renderer2D::Clear(glm::vec3& colour) {
		s_RenderAPI.SetClearColour(colour);
		s_RenderAPI.Clear();
	}

	void Renderer2D::SetViewPort(uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height) {
		s_RenderAPI.SetViewPort(offsetX, offsetY, width, height);
	}

	void Renderer2D::DrawTextureQuad(glm::vec3& position, glm::vec3& scale, glm::vec3& colour, glm::vec2& textureOffset, glm::vec2& textureScale){
		s_RenderAPI.SetDepthBuffer(false);

		m_TextureShader->Bind();
		m_TextureShader->setVec3("colour", colour);
		m_TextureShader->setVec2("texTranslate", textureOffset);
		m_TextureShader->setVec2("texScale", textureScale);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::scale(transform, scale);

		m_TextureShader->setMat4("transform", m_ViewProjection * transform);

		m_QuadVertexArray->Bind();
		s_RenderAPI.DrawIndexed(*m_QuadVertexArray, 6);
	}

	void Renderer2D::DrawFlatQuad(glm::vec3& position, glm::vec3& scale, glm::vec3& colour) {
		s_RenderAPI.SetDepthBuffer(false);
		
		m_FlatShader->Bind();
		m_FlatShader->setVec3("colour", colour);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::scale(transform, scale);

		m_FlatShader->setMat4("transform", m_ViewProjection * transform);

		m_QuadVertexArray->Bind();
		s_RenderAPI.DrawIndexed(*m_QuadVertexArray, 6);
	}

	void Renderer2D::DrawRotatedFlatQuad(glm::vec3& position, float rotation, glm::vec3& rotationAxis, glm::vec3& scale, glm::vec3& colour) {
		s_RenderAPI.SetDepthBuffer(false);
		m_FlatShader->Bind();
		m_FlatShader->setVec3("colour", colour);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::rotate(transform, glm::radians(rotation), rotationAxis);
		transform = glm::scale(transform, scale);

		m_FlatShader->setMat4("transform", m_ViewProjection * transform);

		m_QuadVertexArray->Bind();
		s_RenderAPI.DrawIndexed(*m_QuadVertexArray, 6);
	}

	void Renderer2D::DrawLine(glm::vec3& position, glm::vec3& scale, glm::vec3& colour) {
		s_RenderAPI.SetDepthBuffer(false);
		
		m_FlatShader->Bind();
		m_FlatShader->setVec3("colour", colour);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::scale(transform, scale);
		m_FlatShader->setMat4("transform", m_ViewProjection * transform);

		m_LineVertexArray->Bind();
		s_RenderAPI.DrawLines(*m_LineVertexArray, 2);
	}

	void Renderer2D::SetLineWidth(uint32_t width) {
		s_RenderAPI.SetLineWidth(width);
	}
}