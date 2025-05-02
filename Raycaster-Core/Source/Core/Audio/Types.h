#pragma once

#include <cstdint>
#include <limits>
#include <variant>

namespace Core::Audio::Internal {
    struct BusObject;
    struct SoundObject;

    struct Delay;
    struct Biquad;

    using FilterNode = std::variant<Delay*, Biquad*>;
}

namespace Core::Audio {
    struct Index {
        uint32_t Epoch = std::numeric_limits<uint32_t>::max();
        uint32_t Value = std::numeric_limits<uint32_t>::max();

        operator bool() {
            return (Epoch != std::numeric_limits<uint32_t>::max() || Value != std::numeric_limits<uint32_t>::max());
        }
    };

    class Bus;
    class Sound;

    using ChildNode = std::variant<Bus*, Sound*>;
    
    namespace Effects {
        class Filter;
    }
}