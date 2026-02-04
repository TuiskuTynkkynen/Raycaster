#include "KeyBinds.h"

#include "Core/Serialization/Enumerations.h"

template <>
struct Core::Serialization::Mapping<Settings::KeyBinds::Name> {
    using enum Settings::KeyBinds::Name;
    static inline EnumMapType<Settings::KeyBinds::Name> Value = {
        { Forward,  "Forward" },
        { Backward, "Backward" },
        { Left,     "Left" },
        { Right,    "Right" },
        { LookUp,   "LookUp"},
        { LookDown, "LookDown"},
        { LookLeft, "LookLeft"},
        { LookRight,"LookRight"},
        { Interact, "Interact" },
        { UseItem,  "UseItem" },
        { Item0,    "Item0" },
        { Item1,    "Item1" },
        { Item2,    "Item2" },
    };
};

template<>
inline bool Core::Serialization::Serialize(const Settings::KeyBinds::InputCode& value, Archive& archive) {
    return archive.WriteBytes(std::as_bytes(std::span{ &value, 1 }));
}

template <>
inline std::optional<Settings::KeyBinds::InputCode> Core::Serialization::Deserialize(Archive& archive) {
    return archive.Read<uint32_t>()
        .and_then([](uint32_t value) {
        Settings::KeyBinds::InputCode code(value);
        return code.HasValue() ? code : std::optional<Settings::KeyBinds::InputCode>(std::nullopt);
            });
}

namespace Settings {
    std::optional<KeyBinds::Name> KeyBinds::KeyCodeToKeyBind(KeyCode keyCode) {
        InputCode code(keyCode);
        return code.HasKey() ? InputCodeToKeyBind(code) : std::nullopt;
    }

    std::optional<KeyBinds::Name> KeyBinds::ButtonCodeToKeyBind(ButtonCode buttonCode) {
        InputCode code(buttonCode);
        return code.HasButton() ? InputCodeToKeyBind(code) : std::nullopt;
    }

    std::optional<KeyBinds::Name> KeyBinds::InputCodeToKeyBind(InputCode code) {
        for (uint8_t i = KeyBinds::ENUMERATION_MIN; i <= KeyBinds::ENUMERATION_MAX; i++) {
            if (s_KeyBinds[i].InputCode == code) {
                return static_cast<Name>(i);
            }
        }

        return std::nullopt;
    }

    bool KeyBinds::Serialize(Core::Serialization::Archive& archive) {
        size_t count = 0;
        size_t countPosition = archive.GetPosition();
        bool success = archive.Write<size_t>(0);

        for (uint8_t i = KeyBinds::ENUMERATION_MIN; success && i <= KeyBinds::ENUMERATION_MAX; i++) {
            success &= archive.Write(static_cast<KeyBinds::Name>(i));
            success &= archive.Write(s_KeyBinds[i].InputCode);
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

            if (auto key = archive.Read<InputCode>()) {
                s_KeyBinds[index.value()].InputCode = key.value();
                successCount++;
            }
        }

        return successCount == count.value();
    }
}