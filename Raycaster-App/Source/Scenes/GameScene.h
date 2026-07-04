#pragma once

#include "RaycasterScene.h"
#include "RaycasterEvents.h"

#include "Core/Base/Application.h"
#include "Core/Events/WindowEvent.h"

#include <memory>

class GameScene final : public RaycasterScene {
public:
    void OnAttach(Core::Application& app) override { RaycasterScene::OnAttach(app); };
    void OnDetach(Core::Application& app) override { RaycasterScene::OnDetach(app); };

    void OnUpdate(Core::Timestep deltaTime) override;
    void OnEvent(Core::Event& event) override;
private:
    void SetState(State state) override;

    bool OnRestart(Restart& event);
    bool OnResume(Resume& event);
    bool OnKeyReleased(Core::KeyReleased& event);
};