#pragma once

#include "Core/Base/Timestep.h"
#include "Core/Events/Event.h"
#include "Core/Raycaster/Scene.h"

#include <memory>

namespace Core {
	class Layer {
	protected:
		std::shared_ptr<Core::Scene> m_Scene;
		bool m_Enabled;
	public: 
		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
		virtual void OnUpdate(Timestep deltaTime) = 0;
		virtual void OnEvent(Event& event) = 0;

		void SetEnabled(bool isEnabled) { m_Enabled = isEnabled; }
		inline void SetScene(std::shared_ptr<Core::Scene> scene) { m_Scene = scene; }
	};
}