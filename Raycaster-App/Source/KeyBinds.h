#pragma once

#include "Core/Base/InputCodes.h"
#include "Core/Serialization/Archive.h"

#include <cstdint>
#include <string_view>
#include <array>
#include <optional>

class KeyBind {
public:
    KeyBind() = default;
    constexpr KeyBind(std::string_view name, int32_t defaultKey) 
        : Name(name), KeyCode(defaultKey), DefaultKeyCode(defaultKey) {}


    void Reset() { KeyCode = DefaultKeyCode; }

    uint32_t KeyCode = -1;
    inline std::string_view GetName() { return Name; }
    inline int32_t GetDefaultKeyCode() { return DefaultKeyCode; }
private:
    std::string_view Name = "";
    uint32_t DefaultKeyCode = -1;
};

namespace KeyBinds {
    enum Name : uint8_t {
        Forward = 0,
        Backward,
        Left,
        Right,
        LookUp,
        LookDown,
        Interact,
        UseItem,
        Item0,
        Item1,
        Item2,
        ENUMERATION_MAX = Item2,
        ENUMERATION_MIN = 0,
    };

    std::optional<KeyBinds::Name> KeyCodeToKeyBind(uint32_t keyCode);

    bool Serialize(Core::Serialization::Archive& archive);
    bool Deserialize(Core::Serialization::Archive& archive);
}

inline constinit std::array<KeyBind, KeyBinds::ENUMERATION_MAX + 1> s_KeyBinds = []{
    std::array<KeyBind, KeyBinds::ENUMERATION_MAX + 1> keyBinds;

    keyBinds[KeyBinds::Forward]     = KeyBind("Move Forward",   RC_KEY_W);
    keyBinds[KeyBinds::Backward]    = KeyBind("Move Backward",  RC_KEY_S);
    keyBinds[KeyBinds::Left]        = KeyBind("Turn Left",      RC_KEY_A);
    keyBinds[KeyBinds::Right]       = KeyBind("Turn Right",     RC_KEY_D);
    keyBinds[KeyBinds::LookUp]      = KeyBind("Look Up",        RC_KEY_UP);
    keyBinds[KeyBinds::LookDown]    = KeyBind("Look Down",      RC_KEY_DOWN);
    keyBinds[KeyBinds::Interact]    = KeyBind("Interact",       RC_KEY_SPACE);
    keyBinds[KeyBinds::UseItem]     = KeyBind("Use Item",       RC_KEY_Q);
    keyBinds[KeyBinds::Item0]       = KeyBind("Empty hand",     RC_KEY_0);
    keyBinds[KeyBinds::Item1]       = KeyBind("Item 1",         RC_KEY_1);
    keyBinds[KeyBinds::Item2]       = KeyBind("Item 2",         RC_KEY_2);

    return keyBinds;
}();