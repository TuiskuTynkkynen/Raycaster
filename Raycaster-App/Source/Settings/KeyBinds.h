#pragma once

#include "InputCode.h"

#include "Core/Base/InputCodes.h"
#include "Core/Serialization/Archive.h"

#include <cstdint>
#include <string_view>
#include <array>

namespace Settings::KeyBinds {
    class KeyBind {
    public:
        KeyBind() = default;
        constexpr KeyBind(std::string_view name, InputCode defaultInput)
            : m_Name(name), m_InputCode(defaultInput), m_Default(defaultInput), m_Saved(defaultInput) {}

        inline void Update(KeyBinds::InputCode value) { m_InputCode = value; }
        inline void Reset() { m_InputCode = m_Default; }

        inline std::string_view GetName() const { return m_Name; }
        inline KeyBinds::InputCode GetValue() const { return m_InputCode; }

        inline bool Default() const { return m_InputCode == m_Default;}
        inline bool Saved() const { return m_InputCode == m_Saved;}
    private:
        std::string_view m_Name = "";
        InputCode m_Default{};
        InputCode m_Saved{};
        InputCode m_InputCode{};

        friend bool Serialize(Core::Serialization::Archive& archive);
        friend bool Deserialize(Core::Serialization::Archive& archive);
    };

    enum Name : uint8_t {
        Forward = 0,
        Backward,
        Left,
        Right,
        LookUp,
        LookDown,
        LookLeft,
        LookRight,
        Interact,
        UseItem,
        Item0,
        Item1,
        Item2,
        ENUMERATION_MAX = Item2,
        ENUMERATION_MIN = 0,
    };

    std::optional<KeyBinds::Name> KeyCodeToKeyBind(KeyCode code);
    std::optional<KeyBinds::Name> ButtonCodeToKeyBind(ButtonCode code);
    std::optional<KeyBinds::Name> InputCodeToKeyBind(InputCode code);

    bool Serialize(Core::Serialization::Archive& archive);
    bool Deserialize(Core::Serialization::Archive& archive);
}
namespace Settings{
    inline constinit std::array<KeyBinds::KeyBind, KeyBinds::ENUMERATION_MAX + 1> s_KeyBinds = [] {
        std::array<KeyBinds::KeyBind, KeyBinds::ENUMERATION_MAX + 1> keyBinds;

        using namespace KeyBinds;
        keyBinds[Forward]   = KeyBind("Move Forward",  { KeyCode(RC_KEY_W) });
        keyBinds[Backward]  = KeyBind("Move Backward", { KeyCode(RC_KEY_S) });
        keyBinds[Left]      = KeyBind("Move Left",     { KeyCode(RC_KEY_A) });
        keyBinds[Right]     = KeyBind("Move Right",    { KeyCode(RC_KEY_D) });
        keyBinds[LookUp]    = KeyBind("Look Up",       { KeyCode(RC_KEY_UP) });
        keyBinds[LookDown]  = KeyBind("Look Down",     { KeyCode(RC_KEY_DOWN) });
        keyBinds[LookLeft]  = KeyBind("Look Left",     { KeyCode(RC_KEY_LEFT) });
        keyBinds[LookRight] = KeyBind("Look Right",    { KeyCode(RC_KEY_RIGHT) });
        keyBinds[Interact]  = KeyBind("Interact",      { KeyCode(RC_KEY_SPACE) });
        keyBinds[UseItem]   = KeyBind("Use Item",      { KeyCode(RC_KEY_Q) });
        keyBinds[Item0]     = KeyBind("Empty hand",    { KeyCode(RC_KEY_0) });
        keyBinds[Item1]     = KeyBind("Item 1",        { KeyCode(RC_KEY_1) });
        keyBinds[Item2]     = KeyBind("Item 2",        { KeyCode(RC_KEY_2) });

        return keyBinds;
    }();
}
