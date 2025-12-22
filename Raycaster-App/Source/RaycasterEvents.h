#pragma once

#include "Core/Events/Event.h"

enum class EventType : uint8_t {
    None = static_cast<uint8_t>(Core::EventType::Custom),
    Restart,
};

class Restart : public Core::Event {
public:
    Restart() {}

    static Core::EventType GetStaticType() { return static_cast<Core::EventType>(EventType::Restart); }
    Core::EventType GetType() const override { return GetStaticType(); }
    int GetCategory() const override { return 0; }

};
