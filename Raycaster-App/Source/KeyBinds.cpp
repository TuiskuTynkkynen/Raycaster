#include "KeyBinds.h"

std::optional<KeyBinds::Name> KeyBinds::KeyCodeToKeyBind(uint32_t keyCode) {
    for (uint8_t i = KeyBinds::ENUMERATION_MIN; i <= KeyBinds::ENUMERATION_MAX; i++) {
        if (keyCode == s_KeyBinds[i].KeyCode) {
            return static_cast<Name>(i);
        }
    }

    return std::nullopt;
}