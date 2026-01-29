#pragma once

#include "Entities.h"

#include "Core/Renderer/Model.h"

#include <vector>
#include <span>

struct Model : public Core::Model {
    glm::vec3 Position{};
    glm::vec3 Scale{};

    Model() = default;
    Model(Core::Model model) : Core::Model(model) {}
};

class Renderables {
public:
    void Init(std::shared_ptr<Core::Shader> shader, std::shared_ptr<Core::Texture2D> textureAtlas, std::shared_ptr<Core::Texture2D> mapTexture, size_t capacity = 16);
    void Shutdown();
    
    void ResetDynamic();
    void UpdateDynamicRender(float billboardAngle);

    Model& GetNextModel();
    Sprite& GetNextSprite();

    void PushStaticModel();
    void PushStaticModel(Core::Model model);
    void ResetStaticModels();
    
    size_t GetModelCount() const { return m_Models.size(); }
    size_t GetStaticModelCount() const { return m_StaticModelCount; }
    size_t GetDynamicModelCount() const { return m_ModelBack - m_StaticModelCount; }
    size_t GetSpriteCount() const { return m_Sprites.size(); }

    inline std::span<Model> GetModels() { return { m_Models.begin(), m_Models.begin() + m_ModelBack + 1 }; }
    inline std::span<const Model> GetModels() const {  return { m_Models.begin(), m_Models.begin() + m_ModelBack + 1 }; }
    inline std::span<Model> GetStaticModels() { return { m_Models.begin(), m_Models.begin() + m_StaticModelCount }; }
    inline std::span<Model> GetDynamicModels() { return { m_Models.begin() + m_StaticModelCount, m_Models.begin() + m_ModelBack + 1 }; }
    inline std::span<Sprite> GetSprites() { return { m_Sprites.begin(), m_Sprites.begin() + m_SpriteBack + 1 }; }
private:
    std::shared_ptr<Core::Mesh> m_BillboardMesh;
    std::shared_ptr<Core::Shader> m_Shader;
    std::shared_ptr<Core::Texture2D> m_TextureAtlas;
    std::shared_ptr<Core::Texture2D> m_MapTexture;
    
    std::shared_ptr<Core::Mesh> CreateBillboardMesh();
    Core::Model CreateBillboard();

    std::vector<Model> m_Models;
    std::vector<Sprite> m_Sprites;

    size_t m_StaticModelCount = 0;

    size_t m_ModelBack = -1;
    size_t m_SpriteBack = -1;
};
