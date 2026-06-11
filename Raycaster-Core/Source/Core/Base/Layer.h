#pragma once

#include "Core/Base/Timestep.h"
#include "Core/Events/Event.h"
#include "Core/Scene/Scene.h"

#include <memory>

namespace Core {
    class Layer {
    protected:
        std::shared_ptr<Core::Scene> m_Scene;
    public: 
        virtual ~Layer() {}

        virtual void OnAttach() = 0;
        virtual void OnDetach() = 0;
        virtual void OnUpdate(Timestep deltaTime) = 0;
        virtual void OnEvent(Event& event) = 0;

        inline void SetScene(std::weak_ptr<const Core::Scene> scene) { m_Scene = scene; }
        inline void SetScene(std::shared_ptr<Core::Scene> scene) { m_Scene = scene; }
    };
}
