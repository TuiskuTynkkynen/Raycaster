#pragma once

#include "Core/Base/Layer.h"
#include "Core/Events/WindowEvent.h"

class Layer3D : public Core::Layer {
public:
	void OnAttach() override;
	void OnDetach() override {};
	void OnUpdate(Core::Timestep deltaTime) override;
	void OnEvent(Core::Event& event) override;
private:
	uint32_t m_ViewPortWidth = -1;
	uint32_t m_ViewPortHeight = -1;

	bool OnWindowResizeEvent(Core::WindowResize& event);
};