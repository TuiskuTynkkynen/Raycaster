#include "Settings.h"

#include "KeyBinds.h"

#include "Core/Serialization/Serialization.h"
#include "Core/Serialization/Enumerations.h"

enum class SettingType : uint8_t {
    Input,
    KeyBinds,
    ENUMERATION_MAX = KeyBinds
};

template <>
struct Core::Serialization::Mapping<SettingType> {
    using enum SettingType;
    static inline EnumMapType<SettingType, 2> Value = {
        { Input,    "Input" },
        { KeyBinds,  "KeyBinds" },
    };
};

namespace Settings {
    static bool SerializeInput(Core::Serialization::Archive& archive) {
        constexpr size_t count = 2;

        bool success = archive.Write<size_t>(count);
        success &= Input::s_FreeLook.Serialize(archive);
        success &= Input::s_MouseLook.Serialize(archive);

        return success;
    }

    static bool DeserializeInput(Core::Serialization::Archive& archive) {
        auto count = archive.Read<size_t>();
        size_t position = archive.GetPosition();

        if (!count || count.value() < 2) {
            return false;
        }

        bool success = Input::s_FreeLook.Deserialize(archive);
        success &= Input::s_MouseLook.Deserialize(archive);

        position += count.value() * sizeof(bool);
        archive.SeekPosition(position);

        return success;
    }

	bool Serialize(Core::Serialization::Archive& archive) {
        constexpr size_t count = static_cast<size_t>(SettingType::ENUMERATION_MAX);

        bool success = archive.Write<size_t>(count);
        success &= archive.Write(SettingType::Input);
        success &= SerializeInput(archive);
        success &= archive.Write(SettingType::KeyBinds);
        success &= KeyBinds::Serialize(archive);

        return success;
	}

    bool Deserialize(Core::Serialization::Archive& archive) {
        auto count = archive.Read<size_t>();
        if (!count) { return false; }
        size_t deserializedCount = 0;
        bool success = true;

        size_t previousPosition = archive.GetPosition();
        auto nextType = archive.Read<SettingType>();
        while (deserializedCount <= count.value() && nextType) {
            deserializedCount++;

            switch (nextType.value()) {
            case SettingType::Input:
                success &= DeserializeInput(archive);
                break;
            case SettingType::KeyBinds:
                success &= KeyBinds::Deserialize(archive);
                break;
            }

            previousPosition = archive.GetPosition();
            nextType = archive.Read<SettingType>();
        }

        archive.SeekPosition(previousPosition);
        return success && count >= static_cast<size_t>(SettingType::ENUMERATION_MAX);
    }
}