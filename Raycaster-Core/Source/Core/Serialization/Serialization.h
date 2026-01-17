#pragma once

#include "Archive.h"

#include <optional>

// These functions must be specialised for a type to serialize it
// Note that pointer types and view types will not work
namespace Core::Serialization {
    template <typename T>
    std::optional<T> Deserialize(Archive& archive) = delete;

    template <typename T>
    bool Serialize(const T& value, Archive& archive) = delete;
}

// Specialisations for commonly used types

#include "Core/Debug/Assert.h"

#include <array>
#include <span>
#include <bit>
#include <cstddef>
#include <string>

namespace Core::Serialization {
    // Arithmetic types
    template <typename T> requires std::is_arithmetic_v<T>
    inline static bool Serialize(const T& value, Archive& archive) {
        return archive.WriteBytes(std::as_bytes(std::span{ &value, 1 }));
    }

    template <typename T> requires std::is_arithmetic_v<T>
    inline static std::optional<T> Deserialize(Archive& archive) {
        std::array<std::byte, sizeof(T)> buffer;

        return archive.ReadBytes(buffer) ? std::optional(std::bit_cast<T>(buffer)) : std::nullopt;
    }

    // String types
    template <>
    inline static bool Serialize(const std::string& value, Archive& archive) {
        if (!archive.Write(value.size())) {
            return false;
        }

        return archive.WriteBytes(std::as_bytes(std::span{ value }));
    }

    template <>
    inline static std::optional<std::string> Deserialize(Archive& archive) {
        return archive.Read<size_t>()
            .and_then([&archive](auto size) {
                std::string string(size, '\0');

                return archive.ReadBytes(std::as_writable_bytes(std::span{ string }))
                    ? std::optional(string) : std::nullopt;
            });
    }

    template <>
    inline static bool Serialize(const std::wstring& value, Archive& archive) {
        if (!archive.Write(value.size())) {
            return false;
        }
        return archive.WriteBytes(std::as_bytes(std::span{ value }));
    }

    template <>
    inline static std::optional<std::wstring> Deserialize(Archive& archive) {
        return archive.Read<size_t>()
            .and_then([&archive](auto size) {
                std::wstring string(size, '\0');

                return archive.ReadBytes(std::as_writable_bytes(std::span{ string }))
                    ? std::optional(string) : std::nullopt;
            });
    }

    // Vectors 
    template <typename T>
    inline constexpr bool is_serializable_vector_v = false;

    template <typename T, typename Allocator>
    inline constexpr bool is_serializable_vector_v<std::vector<T, Allocator>> = Serializable<T>;
    
    template <typename VecT> requires is_serializable_vector_v<VecT>
    inline static bool Serialize(const VecT& value, Archive& archive) {
        if (!archive.Write(value.size())) {
            return false;
        }
        
        return archive.WriteBytes(std::as_bytes(std::span{ value }));
    }

    template <typename VecT> requires is_serializable_vector_v<VecT>
    inline static std::optional<VecT> Deserialize(Archive& archive) {
        return archive.Read<size_t>()
            .and_then([&archive](auto size) {
                VecT vector(size);

                return archive.ReadBytes(std::as_writable_bytes(std::span{ vector })) 
                    ? std::optional(vector) : std::nullopt;
            });
    }

    // Arrays
    template <typename T>
    inline constexpr bool is_serializable_array_v = false;

    template <typename T, size_t Size>
    inline constexpr bool is_serializable_array_v<std::array<T, Size>> = Serializable<T> && std::is_default_constructible_v<T>;
    
    template <typename ArrT> requires is_serializable_array_v<ArrT>
    inline static bool Serialize(const ArrT& value, Archive& archive) {
        return archive.WriteBytes(std::as_bytes(std::span{ value }));
    }

    template <typename ArrT> requires is_serializable_array_v<ArrT>
    inline static std::optional<ArrT> Deserialize(Archive& archive) {
        ArrT array{};
        return archive.ReadBytes(std::as_writable_bytes(std::span{ array }))
            ? std::optional(array) : std::nullopt;
    }
}