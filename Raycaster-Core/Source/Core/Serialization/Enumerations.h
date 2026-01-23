#pragma once

#include "Serialization.h"
#include "Archive.h"
#include "Core/Debug/Assert.h"

#include <initializer_list>
#include <string_view>
#include <type_traits>
#include <limits>

namespace Core::Serialization {
    // Define Mapping<Enum> and Mapping<Enum>::Value to serialize enums
    // 
    // Example:  
    /*
    enum class Enumeration { Foo, Bar }
    
    template <>
    struct Core::Serialization::Mapping<Enumeration> {
        inline static EnumMapType<Enumeration> Value = { { Enumeration::Foo, "foo"}, { Enumeration::Bar, "bar"} };
    };
    */
    // Now Enumeration can be serialized and deserialized with Core::Serialization::Archive.Write and Core::Serialization::Archive.Read

    template <typename T>
    using EnumMapType = std::initializer_list<std::pair<T, const char*>>;

    template <typename T>
    struct Mapping {
        inline static struct {} Value;
    };

    template <typename T>
    concept SerializableEnumeration = requires {
        { Mapping<T>::Value } -> std::convertible_to<EnumMapType<T>>;
    };

    template <SerializableEnumeration Enum>
    constexpr std::optional<std::string_view> GetEnumName(Enum value) {
        for (const auto& pair : Mapping<Enum>::Value) {
            if (value == pair.first) {
                return pair.second;
            }
        }

        RC_WARN("Cannot get the name of enum value that does not appear in the mapping");
        return std::nullopt;
    }

    template <SerializableEnumeration Enum>
    constexpr std::optional<Enum> GetEnumValue(std::string_view name) {
        for (const auto& pair : Mapping<Enum>::Value) {
            if (name == pair.second) {
                return pair.first;
            }
        }

        RC_WARN("Cannot get the value of enum name that does not appear in the mapping");
        return std::nullopt;
    }

    template <SerializableEnumeration Enum>
    inline bool Serialize(const Enum& value, Archive& archive) {
        auto name = GetEnumName(value);
        if (!name || !archive.Write(name.value().size())) {
            return false;
        }

        return archive.WriteBytes(std::as_bytes(std::span{ name.value() }));
    }

    template <SerializableEnumeration Enum>
    inline std::optional<Enum> Deserialize(Archive& archive) {
        return archive.Read<std::string>()
            .and_then(GetEnumValue<Enum>);
    }
}
