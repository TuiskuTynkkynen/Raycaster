#pragma once

#include "Core.h"
#include "RaycasterScene.h"

class RaycasterLayer : public Core::Layer {
public:
	void OnAttach() override {};
	void OnDetach() override {};
	void OnUpdate(Core::Timestep deltaTime) override;
	void OnEvent(Core::Event& event) override {}
};

class Layer2D : public Core::Layer {
public:
	void OnAttach() override {};
	void OnDetach() override {};
	void OnUpdate(Core::Timestep deltaTime) override;
	void OnEvent(Core::Event& event) override {}
};
