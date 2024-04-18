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
    std::vector<glm::vec3> m_Lights;
    float* m_ZBuffer = new float[m_RayCount];
    
    struct SpriteObject {
        glm::vec3 Position;
        glm::vec3 Scale;

        uint32_t AtlasIndex;
        bool FlipTexture;

        SpriteObject() {
            AtlasIndex = 0;
            FlipTexture = false;
        }
    };
    std::vector<SpriteObject> m_SpriteObjects;
    struct Enemy {
        glm::vec3 Position;
        glm::vec3 Scale;

        uint32_t AtlasIndex;

        float Tick;
        float Speed;

        Enemy() {
            AtlasIndex = 0;
            Tick = 0.0f;
            Speed = 0.0f;
        }
    };
    bool m_EnemyMap[s_MapData.size];
    std::vector<Enemy> m_Enemies;
    std::vector<glm::vec4> m_Diagonals;
    std::vector<LineCollider> m_Walls;

    
    float max = 0.0f;
    Core::Player m_Player;
    std::unique_ptr<Core::RaycasterCamera> m_Camera;
    
    bool m_Paused = false;
    
    void ProcessInput(Core::Timestep deltaTime);
    void CastRays();
    void RenderSprites();
    void UpdateEnemies(Core::Timestep deltaTime);
public: 
    void Init() override;
    void Shutdown() override {}

    void OnUpdate(Core::Timestep deltaTime) override;
    void OnEvent(Core::Event& event) override {}

    inline std::vector<Core::Ray>& GetRays() { return m_Rays;  }
    inline std::vector<Core::Line>& GetLines() { return m_Lines;  }
    inline std::vector<Core::Tile>& GetTiles() { return m_Tiles;  }
    inline std::vector<Core::Sprite>& GetSprites() override { return m_Sprites; }

    inline Core::Player& GetPlayer() { return m_Player;  }
    inline Core::RaycasterCamera& GetCamera() { return *m_Camera;  }

    inline uint32_t GetRayCount() { return m_RayCount; }
};

