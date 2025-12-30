#pragma once

#include "Core.h"
#include "RaycasterScene.h"

class RaycasterLayer : public Core::Layer {
private :
	uint32_t m_ViewPortWidth = 600;
	uint32_t m_ViewPortHeight = 600;

	bool OnWindowResizeEvent(Core::WindowResize& event);
public:
	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(Core::Timestep deltaTime) override;
	void OnEvent(Core::Event& event) override;
};

class Layer2D : public Core::Layer {
private :
	uint32_t m_ViewPortWidth = 600;
	uint32_t m_ViewPortHeight = 600;

	bool OnWindowResizeEvent(Core::WindowResize& event);
public:
	void OnAttach() override {};
	void OnDetach() override {};
	void OnUpdate(Core::Timestep deltaTime) override;
	void OnEvent(Core::Event& event) override;
};

class Layer3D : public Core::Layer {
private :
	uint32_t m_ViewPortWidth = 600;
	uint32_t m_ViewPortHeight = 600;

	bool OnWindowResizeEvent(Core::WindowResize& event);
public:
	void OnAttach() override {};
	void OnDetach() override {};
	void OnUpdate(Core::Timestep deltaTime) override;
	void OnEvent(Core::Event& event) override;
};
