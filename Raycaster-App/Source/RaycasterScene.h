#pragma once

#include "Core.h"
#include "Algorithms.h"
#include "Entities.h"
#include "Map.h"
#include "Enemies.h"
#include "Interactables.h"
#include "Renderables.h"
#include "Projectiles.h"
#include "RaycasterEvents.h"

#include <memory>

class RaycasterScene : public Core::Scene {
private:
    Map m_Map{};

    const uint32_t m_RayCount = 500;
    const float m_RayWidth = 2.0f / m_RayCount; // Screen is 2.0f wide/tall

    std::vector<Ray> m_Rays;
    std::vector<Floor> m_Floors;
    std::vector<Line> m_Lines;
    std::vector<Tile> m_Tiles;
    std::vector<glm::vec3> m_Lights;
    float* m_ZBuffer = new float[m_RayCount];
    
    Renderables m_Renderables;

    Projectiles m_Projectiles;
    Interactables m_Interactables;
    Enemies m_Enemies;

    std::vector<LineCollider> m_Walls;

    float max = 0.0f;
    Player m_Player;
    std::unique_ptr<Core::RaycasterCamera> m_Camera;
    std::unique_ptr<Core::FlyCamera> m_Camera3D;

    bool m_Paused = false;

    const bool m_SnappingEnabled = true;

    void Reinit();
    void ProcessInput(Core::Timestep deltaTime);
    void UseItem(Core::Timestep deltaTime);
    bool DamageAreas(std::span<const LineCollider> areas, float areaThickness, float damage);
    void CastRays();
    void CastFloors();
    void RenderSprites();
    void RenderInventory();
    float LightBilinear(glm::vec2 position);

    bool OnRestart(Restart& event);
public: 
    void Init() override;
    void Shutdown() override { m_Enemies.Shutdown(); }

    void OnUpdate(Core::Timestep deltaTime) override;
    void OnEvent(Core::Event& event) override;

    inline std::span<const Ray> GetRays() const { return m_Rays; }
    inline std::span<const Floor> GetFloors() const { return m_Floors; }
    inline std::span<const Line> GetLines() const { return m_Lines; }
    inline std::span<const Tile> GetTiles() const { return m_Tiles; }
    inline std::span<const Core::Model> GetModels() const { return m_Renderables.GetModels(); }

    inline const Player& GetPlayer() const { return m_Player; }
    inline const Core::RaycasterCamera& GetCamera() const { return *m_Camera; }
    inline const Core::FlyCamera& GetCamera3D() const { return *m_Camera3D;  }

    inline uint32_t GetRayCount() const { return m_RayCount; }
};

