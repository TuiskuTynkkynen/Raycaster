#pragma once

#include "Types.h"

namespace Core::Audio::Effects {
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