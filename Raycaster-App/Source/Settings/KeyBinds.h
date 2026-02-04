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
        constexpr KeyBind(std::string_view name, KeyBinds::InputCode defaultInput)
            : Name(name), InputCode(defaultInput), DefaultInputCode(defaultInput) {
        }

        void Reset() { InputCode = DefaultInputCode; }

        KeyBinds::InputCode InputCode{};

        inline std::string_view GetName() { return Name; }
        inline KeyBinds::InputCode GetDefaultInputCode() { return DefaultInputCode; }
    private:
        std::string_view Name = "";
        KeyBinds::InputCode DefaultInputCode{};
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
