#pragma once

#include "Core.h"
#include "Algorithms.h"
#include "Entities.h"
#include "Map.h"
#include "Enemies.h"
#include "Interactables.h"

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
    std::vector<Core::Model> m_Models;
    std::vector<glm::vec3> m_Lights;
    float* m_ZBuffer = new float[m_RayCount];
    
    std::vector<Sprite> m_SpriteObjects;
    
    Interactables m_Interactables;
    Enemies m_Enemies;

    std::vector<LineCollider> m_Walls;

    float max = 0.0f;
    Player m_Player;
    std::unique_ptr<Core::RaycasterCamera> m_Camera;
    std::unique_ptr<Core::FlyCamera> m_Camera3D;

    bool m_Paused = false;

    const bool m_SnappingEnabled = true;

    void ProcessInput(Core::Timestep deltaTime);
    void UseItem(Core::Timestep deltaTime);
    void DamageAreas(std::span<const LineCollider> areas, float areaThickness, float damage);
    void CastRays();
    void CastFloors();
    void RenderSprites();
    void RenderInventory();
    void InitModels();
    void InitInteractables(std::span<const Interactable> interactables);
    float LightBilinear(glm::vec2 position);
public: 
    void Init() override;
    void Shutdown() override { m_Enemies.Shutdown(); }

    void OnUpdate(Core::Timestep deltaTime) override;
    void OnEvent(Core::Event& event) override {}

    inline const std::vector<Ray>& GetRays() const { return m_Rays; }
    inline const std::vector<Floor>& GetFloors() const { return m_Floors; }
    inline const std::vector<Line>& GetLines() const { return m_Lines; }
    inline const std::vector<Tile>& GetTiles() const { return m_Tiles; }
    inline const std::vector<Core::Model>& GetModels() const { return m_Models; }

    inline const Player& GetPlayer() const { return m_Player; }
    inline const Core::RaycasterCamera& GetCamera() const { return *m_Camera; }
    inline const Core::FlyCamera& GetCamera3D() const { return *m_Camera3D;  }

    inline uint32_t GetRayCount() const { return m_RayCount; }
};

