#pragma once

#include "RaycasterScene.h"
#include "RaycasterEvents.h"
#include "KeyBinds.h"

#include <memory>

class DemoScene final : public RaycasterScene {
public:
    void OnAttach(Core::Application& app) override { RaycasterScene::OnAttach(app); };
    void OnDetach(Core::Application& app) override { RaycasterScene::OnDetach(app); };

    void OnUpdate(Core::Timestep deltaTime) override;
    void OnEvent(Core::Event& event) override;
private:
    void SetState(State state) override;

    static constexpr float s_TeleportIntervalSeconds = 8.0f;
    float m_TeleportCooldown = 0.0f;
    static constexpr float s_YawSpeed = 45.0f;
    float m_Yaw = 90.0f;

    bool OnResume(Resume& event);
    bool OnKeyReleased(Core::KeyReleased& event);
    bool OnInputReleased(Settings::KeyBinds::InputCode input);
};