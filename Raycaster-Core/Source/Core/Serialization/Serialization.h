#pragma once

#include <vector>
#include <span>

namespace Core::Serialization {
    class SerializationToken {};
    
    template <typename T>
    T Deserialize(std::span<SerializationToken>) = delete;
    
    template <typename T>
    std::vector<SerializationToken> Serialize(const T&) = delete;
    
    template <typename T>
    concept Serializable = requires(T a)
    {
        Deserialize<T>({});
        Serialize<T>(a);
    };
}