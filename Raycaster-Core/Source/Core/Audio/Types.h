#pragma once

#include <cstdint>
#include <limits>

namespace Core::Audio::Internal {
    struct BusObject;
    struct SoundObject;
}

namespace Core::Audio {
    struct Index {
        uint32_t Epoch = std::numeric_limits<uint32_t>::max();
        uint32_t Value = std::numeric_limits<uint32_t>::max();

        operator bool() {
            return (Epoch != std::numeric_limits<uint32_t>::max() || Value != std::numeric_limits<uint32_t>::max());
        }
    };
}