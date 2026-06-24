#pragma once

#include "RaycasterScene.h"
#include "SettingsUI.h"

#include "Core/Base/Layer.h"
#include "Core/Events/WindowEvent.h"
#include "Core/Renderer/Framebuffer.h"

class UILayer : public Core::Layer {
public:
    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(Core::Timestep deltaTime) override;
    void OnEvent(Core::Event& event) override;
private:
    uint32_t m_ViewPortWidth = -1;
    uint32_t m_ViewPortHeight = -1;

    Settings::UI m_SettingsUI{};

    static constexpr uint32_t m_SampleCount = 8;
    Core::MultisampleFramebuffer m_Framebuffer{ 1, 1, m_SampleCount };

    void PauseScreen(const RaycasterScene& scene);
    bool OnWindowResizeEvent(Core::WindowResize& event);
};