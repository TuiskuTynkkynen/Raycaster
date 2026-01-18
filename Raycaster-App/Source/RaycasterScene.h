#pragma once

#include "Entities.h"
#include "Map.h"
#include "Enemies.h"
#include "Interactables.h"
#include "Renderables.h"
#include "Projectiles.h"
#include "Player.h"
#include "RaycasterEvents.h"
#include "RaycastRenderer.h"

#include "Core/Events/WindowEvent.h"

#include <memory>

class RaycasterScene : public Core::Scene {
public: 
    void Init() override;
    void Shutdown() override;

    void OnUpdate(Core::Timestep deltaTime) override;
    void OnEvent(Core::Event& event) override;

    inline std::span<const Ray> GetRays() const { return m_Renderer.GetRays(); }
    inline std::span<const Floor> GetFloors() const { return m_Renderer.GetFloors(); }
    inline std::span<const Line> GetLines() const { return m_Renderer.GetLines(); }
    inline std::span<const Tile> GetTiles() const { return m_Tiles; }
    inline std::span<const Core::Model> GetModels() const { return m_Renderables.GetModels(); }

    inline const Player& GetPlayer() const { return m_Player; }
    inline const RaycasterCamera& GetCamera() const { return *m_Camera; }
    inline const Core::FlyCamera& GetCamera3D() const { return *m_Camera3D;  }

    enum class State : uint8_t {
        Running = 0,
        Paused,
        Dead,
        Invalid,
    };
    inline State GetState() const { return m_State; }
private:
    Map m_Map{};

    std::vector<Tile> m_Tiles;
    std::vector<glm::vec3> m_Lights;

    RaycastRenderer m_Renderer;
    Renderables m_Renderables;

    Projectiles m_Projectiles;
    Interactables m_Interactables;
    Enemies m_Enemies;

    std::vector<LineCollider> m_Walls;

    Player m_Player;
    std::unique_ptr<RaycasterCamera> m_Camera;
    std::unique_ptr<Core::FlyCamera> m_Camera3D;

    State m_State = State::Invalid;

    void Reinit();
    bool OnRestart(Restart& event);
    bool OnResume(Resume& event);
    bool OnKeyReleased(Core::KeyReleased& event);
    bool OnWindowResize(Core::WindowResize& event);
};