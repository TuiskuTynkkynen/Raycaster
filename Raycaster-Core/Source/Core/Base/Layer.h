#pragma once

#include "Core/Base/Timestep.h"
#include "Core/Events/Event.h"

namespace Core {
	class Layer {
	public: 
		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
		virtual void OnUpdate(Timestep deltaTime) = 0;
		virtual void OnEvent(Event& event) = 0;

		void SetEnabled(bool isEnabled) { m_Enabled = isEnabled; }
	private:
		bool m_Enabled;
	};
}