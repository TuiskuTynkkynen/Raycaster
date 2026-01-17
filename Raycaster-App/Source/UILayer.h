#pragma once

#include "RaycasterScene.h"

#include "Core/Base/Layer.h"
#include "Core/Events/WindowEvent.h"

class UILayer : public Core::Layer {
public:
    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(Core::Timestep deltaTime) override;
    void OnEvent(Core::Event& event) override;
private:
    uint32_t m_ViewPortWidth = -1;
    uint32_t m_ViewPortHeight = -1;

    bool m_ShowKeyBinds = false;
    bool m_SavedKeyBinds = true;
    uint32_t m_SelectedKeyBind = -1;

    void PauseScreen(const RaycasterScene& scene);
    void KeyBindsScreen(const RaycasterScene& scene);

    bool OnKeyReleased(Core::KeyReleased& event);
    bool OnWindowResizeEvent(Core::WindowResize& event);
};