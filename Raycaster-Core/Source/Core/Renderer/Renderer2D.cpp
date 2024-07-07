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
#include <array>

struct QuadVertex {
    glm::vec3 Position{ 0.0f };
    glm::vec3 Colour{ 0.0f };
    glm::vec2 TextureCoords{ 0.0f };
    glm::vec2 AtlasOffset{ 0.0f };

    float TextureIndex = 0;
};

struct SimpleVertex {
    glm::vec3 Position{ 0.0f };
    glm::vec3 Colour{ 0.0f };
};

struct Renderer2DDAta {
    static const uint32_t MaxVertices = 1000 * 4;
    static const uint32_t MaxIndices = 1000 * 6;

    std::unique_ptr<Core::VertexArray> LineVertexArray;
    std::unique_ptr<Core::VertexBuffer> LineVertexBuffer;

    std::unique_ptr<Core::VertexArray> TriangleVertexArray;
    std::unique_ptr<Core::VertexBuffer> TriangleVertexBuffer;

    std::unique_ptr<Core::VertexArray> QuadVertexArray;
    std::unique_ptr<Core::VertexBuffer> QuadVertexBuffer;
    std::unique_ptr<Core::ElementBuffer> QuadElementBuffer;

    uint32_t LineVertexCount = 0;
    std::vector<SimpleVertex> LineVertices;
    std::vector<SimpleVertex>::iterator LineBackIter;

    uint32_t TriangleVertexCount = 0;
    std::vector<SimpleVertex> TriangleVertices;
    std::vector<SimpleVertex>::iterator TriangleBackIter;

    uint32_t QuadIndexCount = 0;
    std::vector<QuadVertex> QuadVertices;
    std::vector<QuadVertex>::iterator QuadBackIter;

    uint32_t BaseQuadVertexCount = 4;
    glm::vec4* BaseQuadVertexPositions = nullptr;
    glm::vec3* BaseQuadTextureCoords = nullptr;

    std::unique_ptr<Core::Shader> TextureShader;
    std::unique_ptr<Core::Shader> SimpleShader;

    std::unique_ptr<Core::Texture2D> TextureAtlas;
    std::unique_ptr<Core::Texture2D> WhiteTexture;

    std::shared_ptr<Core::Font> Font;

    glm::mat4 ViewProjection = glm::mat4(1.0f);

    uint32_t atlasWidth = 0;
    uint32_t atlasHeight = 0;
};

static Renderer2DDAta s_Data;

namespace Core {
    void Renderer2D::Init(){
        s_Data.LineVertices.resize(s_Data.MaxVertices);
        s_Data.LineBackIter = s_Data.LineVertices.begin();

        s_Data.TriangleVertices.resize(s_Data.MaxVertices);
        s_Data.TriangleBackIter = s_Data.TriangleVertices.begin();
        
        s_Data.QuadVertices.resize(s_Data.MaxVertices);
        s_Data.QuadBackIter = s_Data.QuadVertices.begin();

        s_Data.BaseQuadVertexPositions = new glm::vec4[]{
            { 0.5f,  0.5f, 0.0f , 1.0f },
            { 0.5f, -0.5f, 0.0f , 1.0f },
            {-0.5f,  0.5f, 0.0f , 1.0f },
            { -0.5f, -0.5f, 0.0f , 1.0f },
        };

        s_Data.BaseQuadTextureCoords = new glm::vec3[]{
            { 1.0f, 1.0f, 1.0f },
            { 1.0f, 0.0f, 1.0f },
            { 0.0f, 1.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f },
        };

        Core::VertexBufferLayout simpleVertexLayout;
        simpleVertexLayout.Push<float>(3);
        simpleVertexLayout.Push<float>(3);

        s_Data.LineVertexArray = std::make_unique<VertexArray>();;
        s_Data.LineVertexBuffer = std::make_unique<VertexBuffer>(sizeof(SimpleVertex) * s_Data.MaxVertices);
        s_Data.LineVertexArray->AddBuffer(*s_Data.LineVertexBuffer, simpleVertexLayout);

        s_Data.TriangleVertexArray = std::make_unique<VertexArray>();
        s_Data.TriangleVertexBuffer = std::make_unique<VertexBuffer>(sizeof(SimpleVertex) * s_Data.MaxVertices);
        s_Data.TriangleVertexArray->AddBuffer(*s_Data.TriangleVertexBuffer, simpleVertexLayout);

        s_Data.QuadVertexArray = std::make_unique<VertexArray>();
        s_Data.QuadVertexBuffer = std::make_unique<VertexBuffer>(sizeof(QuadVertex) * s_Data.MaxVertices);

        VertexBufferLayout quadLayout;
        quadLayout.Push<float>(3);
        quadLayout.Push<float>(3);
        quadLayout.Push<float>(2);
        quadLayout.Push<float>(2);
        quadLayout.Push<float>(1);

        s_Data.QuadVertexArray->AddBuffer(*s_Data.QuadVertexBuffer, quadLayout);

        uint32_t offset = 0;
        uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];
        for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6) {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;
            quadIndices[i + 3] = offset + 1;
            quadIndices[i + 4] = offset + 2;
            quadIndices[i + 5] = offset + 3;

            offset += 4;
        }
        
        s_Data.QuadElementBuffer = std::make_unique<ElementBuffer>(quadIndices, s_Data.MaxIndices);
        delete[] quadIndices;

        s_Data.TextureShader = std::make_unique<Shader>("2DShader.glsl");
        s_Data.TextureShader->Bind();
        
        s_Data.TextureAtlas = std::make_unique<Texture2D>(GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
        s_Data.WhiteTexture = std::make_unique<Texture2D>(GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
        {
            unsigned char missingTextureData[] = {
                0, 0, 0, 255,
                255, 0, 255, 255,
                255, 0, 255, 255,
                0, 0, 0, 255,
            };

            s_Data.TextureAtlas->BindData(missingTextureData, 2, 2, 4);
            
            unsigned char whiteTextureData[] = {
                255, 255, 255, 255,
            };

            s_Data.WhiteTexture->BindData(whiteTextureData, 1, 1, 4);
        }

        s_Data.TextureShader->setInt("Textures[0]", 0);
        s_Data.TextureShader->setInt("Textures[1]", 1);
        s_Data.TextureShader->setInt("Textures[2]", 2);

        s_Data.atlasWidth = s_Data.atlasHeight = 1;
        s_Data.TextureShader->setVec2("AtlasSize", glm::vec2(s_Data.atlasWidth, s_Data.atlasHeight));

        s_Data.SimpleShader = std::make_unique<Shader>("2DSimpleShader.glsl");

        s_Data.Font = std::make_shared<Font>();

        s_Data.ViewProjection = glm::mat4(1.0f);
    }

    void Renderer2D::BeginScene(const Camera& camera) {
        BeginScene(camera.GetViewMatrix());
}

    void Renderer2D::BeginScene(const glm::mat4& transform) {
        s_Data.ViewProjection = transform;

        s_Data.TextureShader->Bind();
        s_Data.TextureShader->setMat4("ViewProjection", s_Data.ViewProjection);

        s_Data.SimpleShader->Bind();
        s_Data.SimpleShader->setMat4("ViewProjection", s_Data.ViewProjection);
    }

    void Renderer2D::EndScene() {
        Flush();
    }
    
    void Renderer2D::Flush() {
        RenderAPI::SetDepthBuffer(false);

        if (s_Data.LineVertexCount != 0) {
            s_Data.LineVertexBuffer->SetData(s_Data.LineVertices.data(), sizeof(SimpleVertex) * s_Data.LineVertexCount);

            s_Data.SimpleShader->Bind();
            RenderAPI::DrawLines(*s_Data.LineVertexArray, s_Data.LineVertexCount);

            s_Data.LineVertexCount = 0;
            s_Data.LineBackIter = s_Data.LineVertices.begin();
        }
        
        if (s_Data.QuadIndexCount != 0) {
            s_Data.TextureShader->Bind();
            s_Data.WhiteTexture->Activate(0);
            s_Data.TextureAtlas->Activate(1);
            s_Data.Font->ActivateAtlas(2);

            uint32_t vertexCount =  s_Data.QuadBackIter - s_Data.QuadVertices.begin();
            s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertices.data(), sizeof(QuadVertex) * vertexCount);
            
            RenderAPI::DrawIndexed(*s_Data.QuadVertexArray, s_Data.QuadIndexCount);
            
            s_Data.QuadIndexCount = 0;
            s_Data.QuadBackIter = s_Data.QuadVertices.begin();
        }

        if (s_Data.TriangleVertexCount != 0) {
            s_Data.TriangleVertexBuffer->SetData(s_Data.TriangleVertices.data(), sizeof(SimpleVertex) * s_Data.TriangleVertexCount);

            s_Data.SimpleShader->Bind();
            RenderAPI::DrawVertices(*s_Data.TriangleVertexArray, s_Data.TriangleVertexCount);

            s_Data.TriangleVertexCount = 0;
            s_Data.TriangleBackIter = s_Data.TriangleVertices.begin();
        }

        RenderAPI::SetDepthBuffer(true);
    }

    void Renderer2D::DrawQuad(uint32_t textureIndex, const glm::vec3& colour, const glm::mat4& transform, const glm::mat3& textureTransform, const glm::vec2& atlasOffset) {
        if (s_Data.QuadIndexCount >= s_Data.MaxIndices) {
            Flush();
        }

        for (uint32_t i = 0; i < 4; i++) {
            s_Data.QuadBackIter->Position = transform * s_Data.BaseQuadVertexPositions[i];
            s_Data.QuadBackIter->Colour = colour;
            s_Data.QuadBackIter->TextureCoords = textureTransform * s_Data.BaseQuadTextureCoords[i];
            s_Data.QuadBackIter->AtlasOffset = atlasOffset;
            s_Data.QuadBackIter->TextureIndex = textureIndex;
            s_Data.QuadBackIter++;
        }

        s_Data.QuadIndexCount += 6;
    }

    void Renderer2D::DrawTextureQuad(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& colour, const glm::vec2& textureOffset, const glm::vec2& textureScale, uint32_t atlasIndex, float textureRotate){
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
        transform = glm::scale(transform, scale);

        glm::mat3 texTransform = glm::translate(glm::mat3(1.0f), textureOffset);
        texTransform = glm::rotate(texTransform, glm::radians(textureRotate));
        texTransform = glm::scale(texTransform, textureScale);

        DrawQuad(1, colour, transform, texTransform, glm::vec2(atlasIndex % s_Data.atlasWidth, atlasIndex / s_Data.atlasWidth));
    }

    void Renderer2D::DrawFlatQuad(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& colour) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
        transform = glm::scale(transform, scale);

        DrawQuad(0, colour, transform);
    }

    void Renderer2D::DrawRotatedFlatQuad(const glm::vec3& position, float rotation, const glm::vec3& rotationAxis, const glm::vec3& scale, const glm::vec3& colour) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
        transform = glm::rotate(transform, glm::radians(rotation), rotationAxis);
        transform = glm::scale(transform, scale);

        DrawQuad(0, colour, transform);
    }

    void Renderer2D::DrawRotatedFlatTriangle(const glm::vec3& position, float rotation, const glm::vec3& rotationAxis, const glm::vec3& scale, const glm::vec3& colour) {
        if (s_Data.TriangleVertexCount >= s_Data.MaxVertices) {
            Flush();
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
        transform = glm::rotate(transform, glm::radians(rotation), rotationAxis);
        transform = glm::scale(transform, scale);

        for (uint32_t i = 0; i < 3; i++) {
            s_Data.TriangleBackIter->Position = transform * s_Data.BaseQuadVertexPositions[i];
            s_Data.TriangleBackIter->Colour = colour;
            s_Data.TriangleBackIter++;
        }

        s_Data.TriangleVertexCount += 3;
    }

    void Renderer2D::DrawLine(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& colour) {
        if (s_Data.LineVertexCount >= s_Data.MaxVertices) {
            Flush();
        }

        s_Data.LineBackIter->Position = position;
        s_Data.LineBackIter->Colour = colour;
        s_Data.LineBackIter++;

        s_Data.LineBackIter->Position = position + scale;
        s_Data.LineBackIter->Colour = colour;
        s_Data.LineBackIter++;

        s_Data.LineVertexCount += 2;
    }

    template <typename T>
    void Renderer2D::DrawString(const T& text, float x, float y, float scale, const glm::vec3& colour) {
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
            
            glm::mat3 texTransform = glm::translate(glm::mat3(1.0f), glyph.TexPosition);
            texTransform = glm::scale(texTransform, glyph.TexScale);
            
            DrawQuad(2, colour, transform, texTransform);
            x += glyph.Advance * scale;
        }
    }

    void Renderer2D::SetTextureAtlas(const char* fileName, uint32_t width, uint32_t height) {
        s_Data.TextureShader->Bind();
        s_Data.TextureAtlas->BindImage(fileName);

        s_Data.atlasWidth = width;
        s_Data.atlasHeight = height;
        s_Data.TextureShader->setVec2("AtlasSize", glm::vec2(s_Data.atlasWidth, s_Data.atlasHeight));
    }

    void Renderer2D::SetFont(std::shared_ptr<Font> font) {
        s_Data.Font = font;
    }

    template void Renderer2D::DrawString<std::string>(const std::string&, float, float, float, const glm::vec3&);
    template void Renderer2D::DrawString<std::wstring>(const std::wstring&, float, float, float, const glm::vec3&);
}

