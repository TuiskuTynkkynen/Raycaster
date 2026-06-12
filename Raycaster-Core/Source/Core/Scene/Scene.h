#pragma once

#include "Core/Base/Timestep.h"
#include "Core/Events/Event.h"

namespace Core {
    class Scene	{
    public:
        virtual ~Scene() {}

        virtual void OnAttach(class Application& application) = 0;
        virtual void OnDetach(class Application& application) = 0;

        virtual void OnUpdate(Timestep deltaTime) = 0;
        virtual void OnEvent(Event& event) = 0;
    };
}

