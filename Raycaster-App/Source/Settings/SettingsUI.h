#pragma once

#include "Core/Events/Event.h"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <array>

namespace Settings {
    class UI {
    public:
        bool IsEnabled = false;

        void Init();
        void Shutdown() {}

        void Render();

        void OnEvent(Core::Event& event);
    private:
        union SettingState {
            struct {
                bool Saved;
                bool Default;
            };
            glm::bvec2 State{ true };
        };

        SettingState RenderVideo();
        SettingState RenderInput();
        SettingState RenderKeyBinds();
        bool OnKeyReleased(Core::KeyReleased& event);
        bool OnButtonPressed(Core::MouseButtonPressed& event);

        uint32_t m_SelectedKeyBind = -1;

        std::array<glm::vec4, 3> m_Deselected{};
        std::array<glm::vec4, 3> m_Selected{};
    };
}