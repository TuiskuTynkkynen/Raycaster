#pragma once

#include <vector>
#include <span>
#include <cstddef>

namespace Core::Serialization {
    class Archive;

    template <typename T>
    T Deserialize(Archive& archive) = delete;

    template <typename T>
    bool Serialize(const T& value, Archive& archive) = delete;

    template <typename T>
    concept Serializable = requires(T a, Archive& archive)
    {
        Deserialize<T>(archive);
        Serialize<T>(a, archive);
    };

    class Archive {
    public:
        template <Serializable T>
        T Read();
        std::span<std::byte> Read(size_t size);

        template <Serializable T>
        bool Write(const T& value);
        bool Write(std::span<std::byte>);
    };
}