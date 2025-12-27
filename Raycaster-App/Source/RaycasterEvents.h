#pragma once

#include "Core/Events/Event.h"

enum class EventType : uint8_t {
    None = static_cast<uint8_t>(Core::EventType::Custom),
    Restart,
    Resume,
};

class Restart : public Core::Event {
public:
    Restart() {}

    static Core::EventType GetStaticType() { return static_cast<Core::EventType>(EventType::Restart); }
    Core::EventType GetType() const override { return GetStaticType(); }
    int GetCategory() const override { return Core::EventCategoryCustom; }
};

class Resume : public Core::Event {
public:
    Resume() {}

    inline static constexpr Core::EventType GetStaticType() { return static_cast<Core::EventType>(EventType::Resume); }
    inline constexpr Core::EventType GetType() const override { return GetStaticType(); }
    inline constexpr int GetCategory() const override { return Core::EventCategoryCustom; }
};
