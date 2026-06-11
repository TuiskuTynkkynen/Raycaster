#pragma once

#include "Core/Base/Timestep.h"
#include "Core/Events/Event.h"
#include "Core/Scene/Scene.h"

#include <memory>

namespace Core {
    class Layer {
    public: 
        virtual ~Layer() {}

        virtual void OnAttach() = 0;
        virtual void OnDetach() = 0;
        virtual void OnUpdate(Timestep deltaTime) = 0;
        virtual void OnEvent(Event& event) = 0;

        inline void SetScene(std::weak_ptr<const Core::Scene> scene) { m_Scene = scene; }
    protected:
        std::weak_ptr<const Core::Scene> m_Scene;
    };
}
