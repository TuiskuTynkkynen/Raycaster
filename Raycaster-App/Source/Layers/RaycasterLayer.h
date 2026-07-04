#pragma once

#include "Core/Base/Layer.h"

class RaycasterLayer : public Core::Layer {
public:
	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(Core::Timestep deltaTime) override;
	void OnEvent(Core::Event& event) override {};
};
