#include "KeyBinds.h"

#include "Core/Serialization/Enumerations.h"

std::optional<KeyBinds::Name> KeyBinds::KeyCodeToKeyBind(uint32_t keyCode) {
    for (uint8_t i = KeyBinds::ENUMERATION_MIN; i <= KeyBinds::ENUMERATION_MAX; i++) {
        if (keyCode == s_KeyBinds[i].KeyCode) {
            return static_cast<Name>(i);
        }
    }

    return std::nullopt;
}

template <>
struct Core::Serialization::Mapping<KeyBinds::Name> {
    using enum KeyBinds::Name;
    static inline EnumMapType<KeyBinds::Name> Value = {
        { Forward,  "Forward" },
        { Backward, "Backward" },
        { Left,     "Left" },
        { Right,    "Right" },
        { Interact, "Interact" },
        { UseItem,  "UseItem" },
        { Item0,    "Item0" },
        { Item1,    "Item1" },
        { Item2,    "Item2" },
    };
};

bool KeyBinds::Serialize(Core::Serialization::Archive& archive) {
    size_t count = 0;
    size_t countPosition = archive.GetPosition();
    bool success = archive.Write<size_t>(0);

    for (uint8_t i = KeyBinds::ENUMERATION_MIN; success && i <= KeyBinds::ENUMERATION_MAX; i++) {
        success &= archive.Write(static_cast<KeyBinds::Name>(i));
        success &= archive.Write(s_KeyBinds[i].KeyCode);
        count++;
    }

    size_t endPosition = archive.GetPosition();
    archive.SeekPosition(countPosition);
    archive.Write(count);
    archive.SeekPosition(endPosition);

    return success;
}

bool KeyBinds::Deserialize(Core::Serialization::Archive& archive) {
    auto count = archive.Read<size_t>();
    
    if (!count) {
        return false;
    }

    size_t successCount = 0;
    for (size_t i = 0; i < count.value(); i++) {
        auto index = archive.Read<KeyBinds::Name>();
        if (!index || index.value() >= s_KeyBinds.size()) {
            continue;
        }
        
        if (auto key = archive.Read<uint32_t>()) {
            s_KeyBinds[index.value()].KeyCode = key.value();
            successCount++;
        }
    }

    return successCount == count.value();
}