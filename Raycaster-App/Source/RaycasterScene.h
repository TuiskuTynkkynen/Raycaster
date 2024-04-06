#pragma once

#include "Core.h"
#include "Core/Raycaster/Scene.h"

#include <memory>

class RaycasterScene : public Core::Scene
{
private:
    struct MapData
    {
        static const uint32_t heigth = 6, width = 5;
        static const uint32_t size = heigth * width;

        const uint32_t map[size]{
            1, 1, 1, 1, 1,
            1, 1, 0, 1, 1,
            1, 0, 0, 0, 1,
            1, 0, 0, 0, 1,
            1, 1, 0, 1, 1,
            1, 1, 1, 1, 1,
        };

        const float mapScalingFactor = sqrt((float)size) / 1.4f;
        const glm::vec3 mapScale = glm::vec3(1.0f / mapScalingFactor, 1.0f / mapScalingFactor, 0.0f);
    };
    static MapData s_MapData;
    
    const uint32_t m_RayCount = 200;
    const float m_RayWidth = 1.0f / m_RayCount;

    std::vector<Core::Ray> m_Rays;
    std::vector<Core::Line> m_Lines;
    std::vector<Core::FlatQuad> m_Tiles;
    std::vector<Core::Sprite> m_Sprites;
    float* m_ZBuffer = new float[m_RayCount];

    Core::Player m_Player;

    std::unique_ptr<Core::RaycasterCamera> m_Camera;
    
    bool m_Paused = false;

    void ProcessInput(Core::Timestep deltaTime);
    void CastRays();
    void RenderSprites();

public: 
    void Init() override;
    void Shutdown() override {}

    void OnUpdate(Core::Timestep deltaTime) override;
    void OnEvent(Core::Event& event) override {}

    inline std::vector<Core::Ray>& GetRays() { return m_Rays;  }
    inline std::vector<Core::Line>& GetLines() { return m_Lines;  }
    inline std::vector<Core::FlatQuad>& GetQuads() { return m_Tiles;  }

    std::vector<Core::Sprite>& GetSprites() override { return m_Sprites; }

    inline Core::Player& GetPlayer() { return m_Player;  }
    inline Core::RaycasterCamera& GetCamera() { return *m_Camera;  }
};

