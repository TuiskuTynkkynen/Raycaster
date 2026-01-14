#pragma once

#include "Archive.h"

namespace Core::Serialization {
    template <typename T>
    T Deserialize(Archive& archive) = delete;

    template <typename T>
    bool Serialize(const T& value, Archive& archive) = delete;
}