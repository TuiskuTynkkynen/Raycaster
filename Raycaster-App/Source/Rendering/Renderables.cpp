#include "Renderables.h"

#include "Core/Debug/Assert.h"

void Renderables::Init(std::shared_ptr<Core::Shader> shader, std::shared_ptr<Core::Texture2D> textureAtlas, std::shared_ptr<Core::Texture2D> mapTexture, size_t capacity) {
    m_BillboardMesh = CreateBillboardMesh();
    m_Shader = shader;
    m_TextureAtlas = textureAtlas;
    m_MapTexture = mapTexture;

    m_Sprites.reserve(capacity);
    m_Models.reserve(capacity);
    
    m_StaticModelCount = 0;
    m_ModelBack = m_SpriteBack = -1;
}

void Renderables::Shutdown(){
    m_BillboardMesh.reset(); 
    m_Shader.reset(); 
    m_TextureAtlas.reset();
    m_MapTexture.reset();

    m_Sprites.resize(0);
    m_Sprites.shrink_to_fit();

    m_Models.resize(0);
    m_Models.shrink_to_fit();

    m_StaticModelCount = 0;
    m_ModelBack = m_SpriteBack = -1;
}

void Renderables::ResetDynamic() {
    m_ModelBack = m_StaticModelCount - 1;
    m_SpriteBack = -1;
}

Model& Renderables::GetNextModel(){
    m_ModelBack++;

    if(m_ModelBack >= m_Models.size()) {
        m_Models.emplace_back(CreateBillboard());
    }

    return m_Models[m_ModelBack];
}

Sprite& Renderables::GetNextSprite(){
    m_SpriteBack++;

    if (m_SpriteBack >= m_Sprites.size()) {
        m_Sprites.emplace_back();
    }

    return m_Sprites[m_SpriteBack];
}

void Renderables::PushStaticModel() {
    PushStaticModel(CreateBillboard());
}

void Renderables::PushStaticModel(Core::Model model) {
    m_ModelBack++;
    m_StaticModelCount++;

    if (m_StaticModelCount >= m_Models.size()) {
        m_Models.emplace_back(model);
        return;
    }

    if (m_ModelBack == m_Models.size()) {
        m_Models.emplace_back(m_Models[m_StaticModelCount - 1]);
        m_Models[m_StaticModelCount - 1] = model;

        return;
    }

    m_Models[m_ModelBack] = m_Models[m_StaticModelCount - 1];
    m_Models[m_StaticModelCount - 1] = model;
}

void Renderables::ResetStaticModels() {
    for (size_t i = 0; i < m_StaticModelCount; i++) {
        m_Models[i] = m_Models[m_ModelBack];
        m_ModelBack--;
    }

    m_StaticModelCount = 0;
}

void Renderables::UpdateDynamicRender(float billboardAngle) {
    for (auto& model : GetDynamicModels()) {
        model.Transform = glm::translate(glm::mat4(1.0f), glm::vec3(model.Position.x, model.Position.z, model.Position.y));
        model.Transform = glm::rotate(model.Transform, glm::radians(billboardAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        model.Transform = glm::scale(model.Transform, model.Scale);
    }
}

std::shared_ptr<Core::Mesh> Renderables::CreateBillboardMesh() {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    vertices.reserve(4 * 8);
    indices.reserve(6);

    for (uint32_t j = 0; j < 4; j++) {
        float x = (j < 2) ? 0.5f : -0.5f;
        float y = (j % 2) ? 0.5f : -0.5f;

        //position
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(0.0f);

        //normal
        const glm::vec3 normal(0.0f, 0.0f, 1.0f);
        vertices.push_back(normal.x);
        vertices.push_back(normal.y);
        vertices.push_back(normal.z);

        //uv
        vertices.push_back((j >= 2) ? 0.0f : 1.0f);
        vertices.push_back((j % 2 == 0) ? 0.0f : 1.0f);
    }

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(3);
    indices.push_back(2);
    indices.push_back(1);

    auto mesh = std::make_shared<Core::Mesh>();
    mesh->VAO = std::make_unique<Core::VertexArray>();
    mesh->VBO = std::make_unique<Core::VertexBuffer>(vertices.data(), static_cast<uint32_t>(sizeof(float) * vertices.size()));
    Core::VertexBufferLayout wallLayout;

    wallLayout.Push<float>(3);
    wallLayout.Push<float>(3);
    wallLayout.Push<float>(2);
    mesh->VAO->AddBuffer(*mesh->VBO, wallLayout);

    mesh->EBO = std::make_unique<Core::ElementBuffer>(indices.data(), static_cast<uint32_t>(indices.size()));

    return mesh;
}

Core::Model Renderables::CreateBillboard() {
    Core::Model model;
    model.Meshes.emplace_back(m_BillboardMesh, 0);

    auto mat = std::make_shared<Core::Material>();
    mat->Shader = m_Shader;
    mat->MaterialMaps.emplace_back();
    mat->MaterialMaps.back().Texture = m_TextureAtlas;
    mat->MaterialMaps.back().TextureIndex = 0;
    
    mat->MaterialMaps.emplace_back();
    mat->MaterialMaps.back().Texture = m_MapTexture;
    mat->MaterialMaps.back().TextureIndex = 2;

    mat->Parameters.emplace_back(glm::vec2(0.0f), "AtlasOffset");
    mat->Parameters.emplace_back(glm::vec2(0.0f), "FlipTexture");

    model.Materials.push_back(mat);

    return model;
}