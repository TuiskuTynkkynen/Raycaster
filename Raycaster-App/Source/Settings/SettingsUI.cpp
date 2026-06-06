#include "SettingsUI.h"

#include "Settings.h"
#include "KeyBinds.h"
#include "Video.h"

#include "Core/Debug/Log.h"
#include "Core/UI/UI.h"
#include "Core/Serialization/Archive.h"

template <>
struct Core::Serialization::Mapping<Core::WindowMode> {
    using enum Core::WindowMode;
    static inline EnumMapType<Core::WindowMode, 3> Value = {
        { Windowed,   "Windowed"   },
        { Borderless, "Borderless" },
        { Fullscreen, "Fullscreen" },
    };
};


static bool LoadSettings() {
    Core::Serialization::Archive arch(std::string_view("settings.bin"));
    if (Settings::Deserialize(arch)) {
        return true;
    }

    RC_INFO("Setting deserialization was unsuccessful");

    arch.SeekPosition(0);
    Settings::Serialize(arch);
    return false;
}

static bool SaveSettings() {
    Core::Serialization::Archive arch(std::string_view("settings.bin"));
    
    if (Settings::Serialize(arch)) {
        arch.TruncateFile();
        return true;
    }

    RC_INFO("Settings serialization was unsuccessful");
    return false;
}

static bool AddToggleBlock(std::string_view name, bool value) {
    Core::UI::BeginContainer(Core::UI::PositioningType::Offset, { -0.025f, 0.0f, 0.0f }, { 0.95f, 0.125f }, glm::vec4(0.0f), Core::UI::LayoutType::Horizontal);
        Core::UI::Text(name, 1.f, Core::UI::TextAlignment::Left, { 0.85f, 1.0f });
        Core::UI::Toggle(value, { 0.076f, 1.0f });
    Core::UI::EndContainer();
    return value;
}

static void ResetVideoSetting() {
    Settings::Video::WindowMode.Reset();
    Settings::Video::VSync.Reset();
    Settings::Video::LayerRaycaster.Reset();
    Settings::Video::Layer3D.Reset();
    Settings::Video::Layer2D.Reset();
}

namespace Settings {
    void UI::Init() {
        LoadSettings();
        m_Deselected = { Core::UI::DefaultColours[0], Core::UI::DefaultColours[0], Core::UI::DefaultColours[0] };
        m_Selected = { Core::UI::DefaultColours[2], Core::UI::DefaultColours[2], Core::UI::DefaultColours[2] };
    }

    void UI::Render() {
        SettingState state;
        Core::UI::Text("Settings", { 0.5f, 0.125f }, glm::vec4(1.0f));

        static float scrollOffset = 0;
        Core::UI::BeginScrollContainer(scrollOffset, { 0.75f, 0.65f }, true, 1.0f, glm::vec4(0.0f), glm::vec4(0.0f)); {
            state.State &= RenderVideo().State;
            state.State &= RenderInput().State;
            state.State &= RenderKeyBinds().State;
            Core::UI::ScrollBar(scrollOffset, { 0.05f, 1.0f });
        } Core::UI::EndScrollContainer();

        Core::UI::BeginContainer({ 0.75f, 0.125f }, glm::vec4(0.0f), Core::UI::LayoutType::Horizontal); {
            if (Core::UI::Button("Save", { 0.3f, 1.0f }, state.Saved ? m_Deselected : Core::UI::DefaultColours, state.Saved ? m_Selected : Core::UI::DefaultTextColours) && !state.Saved) {
                SaveSettings();
            }

            if (Core::UI::Button("Reset to Defaults", { 0.3f, 1.0f }, state.Default ? m_Deselected : Core::UI::DefaultColours, state.Default ? m_Selected : Core::UI::DefaultTextColours)) {
                for (KeyBinds::KeyBind& bind : s_KeyBinds) {
                    bind.Reset();
                }
                Input::s_MouseLook.Reset();
                Input::s_FreeLook.Reset();
                ResetVideoSetting();
            }

            if (Core::UI::Button("Back", { 0.3f, 1.0f })) {
                IsEnabled = false;
                Core::UI::ResetInteractionElement();
            }
        } Core::UI::EndContainer();
    }

    UI::SettingState UI::RenderVideo() {
        SettingState result{};
        Core::UI::Text("Video", { 0.5f, 0.125f }, glm::vec4(1.0f));

        Core::UI::BeginContainer(Core::UI::PositioningType::Offset, { -0.025f, 0.0f, 0.0f }, { 0.95f, 0.125f }, glm::vec4(0.0f), Core::UI::LayoutType::Horizontal);
            Core::UI::Text("Window Mode", 1.f, Core::UI::TextAlignment::Left, { 0.463f, 1.0f });
            static Core::UI::EnumComboState state{ Video::WindowMode.GetValue() };
            state.SetValue(Video::WindowMode.GetValue());
            Core::WindowMode selected = Core::UI::EnumCombo(state, 5, { 0.463f, 1.0f });

            result.Saved &= Video::WindowMode.Update(selected);
            result.Default &= Video::WindowMode.Default();
        Core::UI::EndContainer();

        bool vSync = AddToggleBlock("Enable VSync", Video::VSync);
        result.Saved &= Video::VSync.Update(vSync);
        result.Default &= Video::VSync.Default();

        bool raycaster = AddToggleBlock("Enable Raycaster View", Video::LayerRaycaster);
        result.Saved &= Video::LayerRaycaster.Update(raycaster);
        result.Default &= Video::LayerRaycaster.Default();

        bool layer3D = AddToggleBlock("Enable 3D View", Video::Layer3D);
        result.Saved &= Video::Layer3D.Update(layer3D);
        result.Default &= Video::Layer3D.Default();

        bool layer2D = AddToggleBlock("Enable 2D View", Video::Layer2D);
        result.Saved &= Video::Layer2D.Update(layer2D);
        result.Default &= Video::Layer2D.Default();
        return result;
    }

    UI::SettingState UI::RenderInput() {
        SettingState result{};
        Core::UI::Text("Input", { 0.5f, 0.125f }, glm::vec4(1.0f));

        Core::UI::BeginContainer(Core::UI::PositioningType::Offset, { -0.025f, 0.0f, 0.0f }, { 0.95f, 0.125f }, glm::vec4(0.0f), Core::UI::LayoutType::Horizontal);
            Core::UI::Text("Enable Mouse Look", 1.f, Core::UI::TextAlignment::Left, { 0.85f, 1.0f });
           
            bool mouseLook = Input::s_MouseLook;
            Core::UI::Toggle(mouseLook, { 0.076f, 1.0f });

            result.Saved &= Input::s_MouseLook.Update(mouseLook);
            result.Default &= Input::s_MouseLook.Default();
        Core::UI::EndContainer();

        Core::UI::BeginContainer(Core::UI::PositioningType::Offset, { -0.025f, 0.0f, 0.0f }, { 0.95f, 0.125f }, glm::vec4(0.0f), Core::UI::LayoutType::Horizontal);
            Core::UI::Text("Enable Free Look", 1.f, Core::UI::TextAlignment::Left, { 0.85f, 1.0f });

            bool freeLook = Input::s_FreeLook;
            Core::UI::Toggle(freeLook, { 0.076f, 1.0f });

            result.Saved &= Input::s_FreeLook.Update(freeLook);
            result.Default &= Input::s_FreeLook.Default();
        Core::UI::EndContainer();
        return result;
    }

    UI::SettingState UI::RenderKeyBinds() {
        SettingState result{};
        Core::UI::Text("Key Bindings", { 0.5f, 0.125f }, glm::vec4(1.0f));

        for (uint32_t i = 0; i < s_KeyBinds.size(); i++) {
        Core::UI::BeginContainer(Core::UI::PositioningType::Offset, { -0.025f, 0.0f, 0.0f }, { 0.95f, 0.125f }, glm::vec4(0.0f), Core::UI::LayoutType::Horizontal);
            Core::UI::Text(s_KeyBinds[i].GetName(), 1.f, Core::UI::TextAlignment::Left, { 0.5f, 1.0f });

            bool isSelected = m_SelectedKeyBind == i;
            if (Core::UI::Button(s_KeyBinds[i].GetValue().ToString(), {0.2f, 1.0f}, isSelected ? m_Selected : Core::UI::DefaultColours)) {
                m_SelectedKeyBind = isSelected ? -1 : i;
            }

            bool isDefault = s_KeyBinds[i].Default();
            result.Default &= isDefault;
            result.Saved &= s_KeyBinds[i].Saved();

            if (Core::UI::Button("Reset", { 0.2f, 1.0f }, isDefault ? m_Deselected : Core::UI::DefaultColours, isDefault ? m_Selected : Core::UI::DefaultTextColours)) {
                s_KeyBinds[i].Reset();
            }
        Core::UI::EndContainer();
        }
        return result;
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
            if (m_SelectedKeyBind == -1) {
                IsEnabled = false;
                Core::UI::ResetInteractionElement();
            }

            m_SelectedKeyBind = -1;
            return true;
        }

        if (m_SelectedKeyBind != -1 && m_SelectedKeyBind < Settings::s_KeyBinds.size()) {
            Settings::s_KeyBinds[m_SelectedKeyBind].Update(event.GetKeyCode());
            m_SelectedKeyBind = -1;
            return true;
        }

        return false;
    }

    bool UI::OnButtonPressed(Core::MouseButtonPressed& event) {
        if (!IsEnabled || m_SelectedKeyBind >= s_KeyBinds.size()) {
            return false;
        }

        s_KeyBinds[m_SelectedKeyBind].Update(event.GetButton());
        m_SelectedKeyBind = -1;
        return true;
    }
}