#pragma once

#include <cstdint>
#include <limits>
#include <variant>

namespace Core::Audio::Internal {
    struct BusObject;
    struct SoundObject;

    struct Delay;
    struct Biquad;
    struct LowPass;
    struct HighPass;
    struct BandPass;
    struct Notch;
    struct PeakingEQ;
    struct LowShelf;
    struct HighShelf;

    using FilterNode = std::variant<Delay*, Biquad*, LowPass*, HighPass*, BandPass*, Notch*, PeakingEQ*, LowShelf*, HighShelf*>;
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