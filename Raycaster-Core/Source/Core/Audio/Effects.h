#pragma once

#include "Types.h"

#include <chrono>
#include <array>

namespace Core::Audio::Effects {
    enum class FilterType : uint8_t {
        Delay = 0,
        Biquad,
        LowPass,
        HighPass,
        BandPass,
        Notch,
        PeakingEQ,
        LowShelf,
        HighShelf,
    };
    
    struct DelaySettings {
        std::chrono::milliseconds Length;

        float Decay = 0.0f; // Echo

        float Dry = 1.0f;
        float Wet = 1.0f;

        bool DelaySoundStart = false; // If true, starting of sounds will be delayed by Length. Automatically set, if decay = 0
    };

    struct BiquadSettings {
        std::array<float, 3> A{}; // Denominator coefficients 
        std::array<float, 3> B{}; // Numerator coefficients 
    };

    struct LowPassSettings {
        double CutoffFrequency;

        uint8_t Order = 2; // Max 8. If 0 no filtering will be applied
    };

    struct HighPassSettings {
        double CutoffFrequency;

        uint8_t Order = 2; // Max 8. If 0 no filtering will be applied
    };

    struct BandPassSettings {
        double CutoffFrequency;

        uint8_t Order = 2; // Must be even, Max 8. If 0 no filtering will be applied
    };

    struct NotchSettings {
        double Frequency;

        double Q = 0.0;
    };

    struct PeakingEQSettings {
        double GainDB;
        double Frequency;

        double Q = 0.0;
    };

    struct LowShelfSettings {
        double GainDB;
        double Slope;
        double Frequency;
    };

    struct HighShelfSettings {
        double GainDB;
        double Slope;
        double Frequency;
    };

    class Filter {
        friend class Bus;
    public:
        template <typename T>
        Filter(T settings, Bus& parent);

        template <typename T>
        Filter(T settings, Filter& parent);

        ~Filter();

        Filter(const Filter&) = delete;
        Filter(Filter&& other) noexcept;

        Filter& operator = (const Filter& other) = delete;
        Filter& operator = (Filter&& other) noexcept;

        bool Reinit();

        FilterType GetType();

        void AttachParent(Bus& parent); // Detach old parent and attach new parent
        void AttachParent(Filter& parent); // Detach old parent and attach new parent
    private:
        Internal::FilterNode m_InternalFilter;

        using RelativeNode = std::variant<Bus*, Filter*, std::nullptr_t>;
        void SwitchParent(RelativeNode parent);

        void AttachChild(RelativeNode child);
        void DetachChild();

        RelativeNode m_Parent;
        RelativeNode m_Child;
    };
}