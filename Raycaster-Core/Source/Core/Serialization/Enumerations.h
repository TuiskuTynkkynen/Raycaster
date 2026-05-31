#pragma once

#include "Core/Debug/Assert.h"

#include <initializer_list>
#include <string_view>
#include <type_traits>
#include <limits>

namespace Core::Serialization {
    // Define Mapping<Enum> and Mapping<Enum>::Value to serialize enums.
    // 
    // Example:  
    /*
    enum class Enumeration { Foo, Bar }
    
    template <>
    struct Core::Serialization::Mapping<Enumeration> {
        inline static EnumMapType<Enumeration> Value = { { Enumeration::Foo, "foo"}, { Enumeration::Bar, "bar"} };
    };
    */
    // Now Enumeration values can be converted to std::string_view and vice versa using 
    // the Core::Serialization::GetEnumName and Core::Serialization::GetEnumValue functions.
    // 
    // This also allows Enumeration to be serialized and deserialized with 
    // the Core::Serialization::Archive.Write and Core::Serialization::Archive.Read functions.
    
    template <typename T, size_t S> class EnumMapType;

    template <typename T>
    struct Mapping {
        inline static struct {} Value;
    };
    
    template <typename T>
    concept SerializableEnumeration = requires {
        { Mapping<T>::Value.Size } -> std::convertible_to<size_t>;
        { Mapping<T>::Value } -> std::convertible_to<EnumMapType<T, Mapping<T>::Value.Size>>;
    };

    template <SerializableEnumeration Enum>
    constexpr std::optional<std::string_view> GetEnumName(Enum value) {
        return Mapping<Enum>::Value.GetName(value);
    }

    template <SerializableEnumeration Enum>
    constexpr std::optional<Enum> GetEnumValue(std::string_view name) {
        return Mapping<Enum>::Value.GetValue(name);
    }
}

template <typename T, size_t S>
class Core::Serialization::EnumMapType {
public:
    inline static constexpr size_t Size = S;
    using value_t = T;
        
    consteval EnumMapType(std::initializer_list<std::pair<T, std::string_view>> map) {
        if (map.size() != Size) { throw "EnumMapType<T, S>::Size and provided std::intilizer_list size must be equal!"; }
        size_t i = 0;
        for (auto& pair : map) {
            m_Values[i] = pair.first;
            m_Names[i++] = pair.second;
        }
    }

    constexpr std::optional<std::string_view> GetName(T value) {
        for (size_t i = 0; i < Size; i++) {
            if (value == m_Values[i]) return m_Names[i];
        }
        RC_WARN("Cannot get the name of enum value that does not appear in the mapping");
        return std::nullopt;
    }

    constexpr std::optional<T> GetValue(std::string_view name) {
        for (size_t i = 0; i < Size; i++) {
            if (name == m_Names[i]) return m_Values[i];
        }
        RC_WARN("Cannot get the value of enum name that does not appear in the mapping");
        return std::nullopt;
    }

    inline constexpr std::span<const T> GetMappedValues() { return m_Values; }
    inline constexpr std::span<const std::string_view> GetMappedNames() { return m_Names; }
private:
    std::array<T, Size> m_Values;
    std::array<std::string_view, Size> m_Names;
};
