#pragma once

#include "Core/Base/Layer.h"
#include "Core/Events/WindowEvent.h"
#include "Core/Renderer/Framebuffer.h"

class Layer3D : public Core::Layer {
public:
	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(Core::Timestep deltaTime) override;
	void OnEvent(Core::Event& event) override;
private:
	Core::MultisampleFramebuffer m_Framebuffer{ 500, 500, 4, Core::MultisampleFramebuffer::ColorFormat::HDR_F };
	std::unique_ptr<Core::Shader> m_PostProcessShader;

	uint32_t m_ViewPortWidth = 0;
	uint32_t m_ViewPortHeight = 0;

	bool OnWindowResizeEvent(Core::WindowResize& event);
};
