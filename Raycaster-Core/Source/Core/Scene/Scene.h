#pragma once

#include "Core/Base/Timestep.h"
#include "Core/Events/Event.h"

namespace Core {
    class Scene	{
    public:
        virtual void Init() = 0;
        virtual void Shutdown() = 0;

        virtual void OnUpdate(Timestep deltaTime) = 0;
        virtual void OnEvent(Event& event) = 0;
    };
}

