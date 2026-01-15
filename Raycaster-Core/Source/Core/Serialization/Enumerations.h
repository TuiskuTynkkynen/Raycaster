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
    constexpr std::string_view GetEnumName(Enum value) {
        for (const auto& pair : Mapping<Enum>::Value) {
            if (value == pair.first) {
                return pair.second;
            }
        }

        RC_ASSERT(false, "Cannot get the name of enum value that does not appear in the mapping");
        return "Invalid Enum Value";
    }

    template <SerializableEnumeration Enum>
    constexpr Enum GetEnumValue(std::string_view name) {
        for (const auto& pair : Mapping<Enum>::Value) {
            if (name == pair.second) {
                return pair.first;
            }
        }

        RC_ASSERT(false, "Cannot get the value of enum name that does not appear in the mapping");
        return static_cast<Enum>(std::numeric_limits<std::underlying_type_t<Enum>>::max());
    }

    template <SerializableEnumeration Enum>
    inline static bool Serialize(const Enum& value, Archive& archive) {
        auto name = GetEnumName(value);
        if (!archive.Write(name.size())) {
            return false;
        }

        return archive.Write(std::as_bytes(std::span{ name }));
    }

    template <SerializableEnumeration Enum>
    inline static Enum Deserialize(Archive& archive) {
        return GetEnumValue<Enum>(archive.Read<std::string>());
    }
}