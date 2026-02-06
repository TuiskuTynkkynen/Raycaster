#include "SettingsUI.h"

#include "KeyBinds.h"

#include "Core/Debug/Log.h"
#include "Core/UI/UI.h"
#include "Core/Serialization/Archive.h"

namespace Settings {
    void UI::Init() {
        Core::Serialization::Archive arch(std::string_view("settings.bin"));
        if (!Settings::KeyBinds::Deserialize(arch)) {
            RC_INFO("Key binding deserialization was unsuccessful");

            arch.SeekPosition(0);
            Settings::KeyBinds::Serialize(arch);
        }

        m_Deselected = { Core::UI::DefaultColours[0], Core::UI::DefaultColours[0], Core::UI::DefaultColours[0] };
        m_Selected = { Core::UI::DefaultColours[2], Core::UI::DefaultColours[2], Core::UI::DefaultColours[2] };
    }

    void UI::Render() {
        Core::UI::Text("Settings", { 0.5f, 0.125f }, glm::vec4(1.0f));

        static float scrollOffset = 0;
        Core::UI::BeginScrollContainer(scrollOffset, { 0.75f, 0.65f }, true, 1.0f, glm::vec4(0.0f), glm::vec4(0.0f)); {
            RenderKeyBinds();
            Core::UI::ScrollBar(scrollOffset, { 0.05f, 1.0f });
        } Core::UI::EndScrollContainer();

        Core::UI::BeginContainer({ 0.75f, 0.125f }, glm::vec4(0.0f), Core::UI::LayoutType::Horizontal); {
            if (Core::UI::Button("Save", { 0.3f, 1.0f }, m_Saved ? m_Deselected : Core::UI::DefaultColours, m_Saved ? m_Selected : Core::UI::DefaultTextColours) && !m_Saved) {
                Core::Serialization::Archive arch(std::string_view("settings.bin"));
                if ((m_Saved = KeyBinds::Serialize(arch))) {
                    arch.TruncateFile();
                }
                else {
                    RC_INFO("Key binding deserialization was unsuccessful");
                }
            }

            if (Core::UI::Button("Reset to Defaults", { 0.3f, 1.0f }, m_Default ? m_Deselected : Core::UI::DefaultColours, m_Default ? m_Selected : Core::UI::DefaultTextColours)) {
                m_Saved &= m_Default;
                for (KeyBinds::KeyBind& bind : s_KeyBinds) {
                    bind.Reset();
                }
            }

            if (Core::UI::Button("Back", { 0.3f, 1.0f })) {
                IsEnabled = false;
            }
        } Core::UI::EndContainer();
    }

    void UI::RenderKeyBinds() {
        Core::UI::Text("Key Binds", { 0.5f, 0.125f }, glm::vec4(1.0f));

        for (uint32_t i = 0; i < s_KeyBinds.size(); i++) {
        Core::UI::BeginContainer(Core::UI::PositioningType::Offset, { -0.025f, 0.0f }, { 0.95f, 0.125f }, glm::vec4(0.0f), Core::UI::LayoutType::Horizontal);
            Core::UI::Text(s_KeyBinds[i].GetName(), 1.f, Core::UI::TextAlignment::Left, { 0.5f, 1.0f });

            bool isSelected = m_SelectedKeyBind == i;
            if (Core::UI::Button(s_KeyBinds[i].InputCode.ToString(), { 0.2f, 1.0f }, isSelected ? m_Selected : Core::UI::DefaultColours)) {
                m_SelectedKeyBind = isSelected ? -1 : i;
            }

            bool isDefault = s_KeyBinds[i].InputCode == s_KeyBinds[i].GetDefaultInputCode();
            m_Default &= isDefault;

            if (Core::UI::Button("Reset", { 0.2f, 1.0f }, isDefault ? m_Deselected : Core::UI::DefaultColours, isDefault ? m_Selected : Core::UI::DefaultTextColours)) {
                m_Saved &= isDefault;
                s_KeyBinds[i].Reset();
            }
        Core::UI::EndContainer();
        }
    }

    void UI::OnEvent(Core::Event& event) {
        Core::EventDispatcher dispatcer(event);
        dispatcer.Dispatch<Core::KeyReleased>([&](auto& e) { return OnKeyReleased(e); });
        dispatcer.Dispatch<Core::MouseButtonPressed>([&](auto& e) { return OnButtonPressed(e); });
    }

    bool UI::OnKeyReleased(Core::KeyReleased& event) {
        if (!IsEnabled) {
            return false;
        }

        if (event.GetKeyCode() == RC_KEY_ESCAPE) {
            IsEnabled = m_SelectedKeyBind != -1;
            m_SelectedKeyBind = -1;
            return true;
        }

        if (m_SelectedKeyBind != -1 && m_SelectedKeyBind < Settings::s_KeyBinds.size()) {
            Settings::s_KeyBinds[m_SelectedKeyBind].InputCode.SetCode(event.GetKeyCode());
            m_SelectedKeyBind = -1;
            m_Saved = false;
            return true;
        }

        return false;
    }

    bool UI::OnButtonPressed(Core::MouseButtonPressed& event) {
        if (!IsEnabled || m_SelectedKeyBind >= s_KeyBinds.size()) {
            return false;
        }

        s_KeyBinds[m_SelectedKeyBind].InputCode.SetCode(event.GetButton());
        m_SelectedKeyBind = -1;
        m_Saved = false;

        return true;
    }
}