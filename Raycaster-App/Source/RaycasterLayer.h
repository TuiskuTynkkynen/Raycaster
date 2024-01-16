#pragma once

#include "Core.h"

class RaycasterLayer : public Core::Layer {
public:

	void OnAttach() override;
	void OnDetach() override {};
	void OnUpdate(Core::Timestep deltaTime) override;
	void OnEvent(Core::Event& event) override {}
private:
	Core::RaycasterCamera* m_Camera;  //Lifetime is application lifetime
};
