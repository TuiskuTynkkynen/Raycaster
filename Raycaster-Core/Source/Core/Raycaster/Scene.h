#pragma once

#include "Core/Base/Timestep.h"
#include "Core/Events/Event.h"
#include "Core/Renderer/Camera.h"
#include "Core/Raycaster/Entities.h"

namespace Core {
class Scene	//TODO switch to data-oriented design/ECS
{
public:
	virtual void Init() = 0;
	virtual void Shutdown() = 0;

	virtual void OnUpdate(Timestep deltaTime) = 0;
	virtual void OnEvent(Event& event) = 0;

	virtual RaycasterCamera& GetCamera() = 0;
	virtual Player& GetPlayer() = 0;

	virtual std::vector<Ray>& GetRays() = 0;
	virtual std::vector<Sprite>& GetSprites() = 0;
	virtual std::vector<Tile>& GetTiles() = 0;
	virtual std::vector<Line>& GetLines() = 0;

	virtual uint32_t GetRayCount() = 0;
};
}

