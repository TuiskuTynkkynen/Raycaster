#pragma once

#include "Core.h"
#include "Core/Raycaster/Scene.h"
#include "Algorithms.h"

#include <memory>

class RaycasterScene : public Core::Scene {
private:
    struct MapData {
        static const uint32_t height = 24, width = 24;
        static const uint32_t size = height * width;
        const int32_t map[size]{ 
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,1,
            1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,-2,2,2,2,-2,0,0,0,0,3,0,3,0,3,0,0,0,1,
            1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1,
            1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,-2,2,0,2,-2,0,0,0,0,3,0,3,0,3,0,0,0,1,
            1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-5,-5,0,0,0,0,0,1,
            1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,-5,-5,0,0,0,0,0,1,
            1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
        };

        const float mapScalingFactor = sqrt((float)size) / 1.4f;
        const glm::vec3 mapScale = glm::vec3(1.0f / mapScalingFactor, 1.0f / mapScalingFactor, 0.0f);
    };
    static MapData s_MapData;
    
    const uint32_t m_RayCount = 500;
    const float m_RayWidth = 1.0f / m_RayCount;

    std::vector<Core::Ray> m_Rays;
    std::vector<Core::Line> m_Lines;
    std::vector<Core::Tile> m_Tiles;
    std::vector<Core::Sprite> m_Sprites;
    std::vector<Core::Model> m_Models;
    std::vector<glm::vec3> m_Lights;
    float* m_ZBuffer = new float[m_RayCount];
    
    struct SpriteObject {
        glm::vec3 Position{};
        glm::vec3 Scale{};

        uint32_t AtlasIndex = 0;
        bool FlipTexture = false;
    };
    std::vector<SpriteObject> m_SpriteObjects;
    std::vector<SpriteObject> m_StaticObjects;
    struct Enemy {
        glm::vec3 Position{};
        glm::vec3 Scale{};

        uint32_t AtlasIndex = 0;

        float Tick = 0.0f;
        float Speed = 0.0f;
    };
    bool m_EnemyMap[s_MapData.size];
    std::vector<Enemy> m_Enemies;
    std::vector<glm::vec4> m_Diagonals;
    std::vector<LineCollider> m_Walls;

    
    float max = 0.0f;
    Core::Player m_Player;
    std::unique_ptr<Core::RaycasterCamera> m_Camera;
    std::unique_ptr<Core::FlyCamera> m_Camera3D;

    bool m_Paused = false;
    
    void ProcessInput(Core::Timestep deltaTime);
    void CastRays();
    void RenderSprites();
    void UpdateEnemies(Core::Timestep deltaTime);
    void InitWalls();
    void InitModels();
public: 
    void Init() override;
    void Shutdown() override {}

    void OnUpdate(Core::Timestep deltaTime) override;
    void OnEvent(Core::Event& event) override {}

    inline const std::vector<Core::Ray>& GetRays() const override { return m_Rays; }
    inline const std::vector<Core::Line>& GetLines() const override { return m_Lines; }
    inline const std::vector<Core::Tile>& GetTiles() const override { return m_Tiles; }
    inline const std::vector<Core::Sprite>& GetSprites() const override { return m_Sprites; }
    inline const std::vector<Core::Model>& GetModels() const override { return m_Models; }

    inline const Core::Player& GetPlayer() const override { return m_Player; }
    inline const Core::RaycasterCamera& GetCamera() const override { return *m_Camera; }
    inline const Core::FlyCamera& GetCamera3D() const override { return *m_Camera3D;  }

    inline uint32_t GetRayCount() { return m_RayCount; }
};

