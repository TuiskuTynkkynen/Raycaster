#pragma once

#include "SettingsUI.h"

#include "Core/Scene/Scene.h"
#include "Core/Renderer/Framebuffer.h"
#include "Core/Renderer/Texture.h"
#include "Core/Events/WindowEvent.h"

#include <array>

class MenuScene final : public Core::Scene {
public:
    MenuScene();

    void OnAttach(Core::Application& app) override;
    void OnDetach(Core::Application& app) override;

    void OnUpdate(Core::Timestep deltaTime) override;
    void OnEvent(Core::Event& event) override;
private:
    uint32_t m_ViewPortWidth = 0;
    uint32_t m_ViewPortHeight = 0;

    Settings::UI m_SettingsUI{};

    static constexpr uint32_t m_SampleCount = 8;
    Core::MultisampleFramebuffer m_Framebuffer{ 1, 1, m_SampleCount };
    std::array<Core::Texture2D, 3> m_Background;

    void MainMenu();
    bool OnWindowResizeEvent(Core::WindowResize& event);
};
