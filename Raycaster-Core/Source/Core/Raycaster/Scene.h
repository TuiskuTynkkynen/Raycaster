#pragma once

#include "Core/Base/Timestep.h"
#include "Core/Events/Event.h"
#include "Core/Renderer/Camera.h"
#include "Core/Renderer/Model.h"
#include "Core/Raycaster/Entities.h"

namespace Core {
class Scene	//TODO switch to data-oriented design/ECS
{
public:
    virtual void Init() = 0;
    virtual void Shutdown() = 0;

    virtual void OnUpdate(Timestep deltaTime) = 0;
    virtual void OnEvent(Event& event) = 0;

    virtual const RaycasterCamera& GetCamera() const = 0;
    virtual const FlyCamera& GetCamera3D() const = 0;
    virtual const Player& GetPlayer() const = 0;

    virtual const std::vector<Ray>& GetRays() const = 0;
    virtual const std::vector<Sprite>& GetSprites() const = 0;
    virtual const std::vector<Tile>& GetTiles() const = 0;
    virtual const std::vector<Line>& GetLines() const = 0;
    virtual const std::vector<Model>& GetModels() const = 0;

    virtual uint32_t GetRayCount() = 0;
};
}

