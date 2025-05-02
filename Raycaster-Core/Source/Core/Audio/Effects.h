#pragma once

#include "Types.h"

#include <chrono>

namespace Core::Audio::Effects {
    struct DelaySettings {
        std::chrono::milliseconds Length;

        float Decay = 0.0f; // Echo

        float Dry = 1.0f;
        float Wet = 1.0f;

        bool DelaySoundStart = false; // If true, starting of sounds will be delayed by Length. Automatically set, if decay = 0
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
        Filter& operator = (const Filter&& other) = delete;

        bool Reinit();

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