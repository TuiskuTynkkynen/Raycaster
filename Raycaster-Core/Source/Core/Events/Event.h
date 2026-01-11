#pragma once

#include <functional>

#define Bit(x) (1 << x)

namespace Core {
    enum class EventType {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus,
        KeyPressed, KeyReleased,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
        TextInput,
        ApplicationClose,
        Custom,
    };

    enum EventCategory {
        None = 0,
        EventCategoryWindow      = Bit(0),
        EventCategoryInput       = Bit(1),
        EventCategoryKeyboard    = Bit(2),
        EventCategoryMouse	     = Bit(3),
        EventCategoryText	     = Bit(4),
        EventCategoryApplication = Bit(5),
        EventCategoryCustom      = Bit(6),
    };

    class Event {
    public:
        virtual ~Event() {}

        bool Handled = false;

        virtual EventType GetType() const = 0;
        virtual int GetCategory() const = 0;

        inline bool IsInCategory(EventCategory category) const {
            return GetCategory() & category;
        }

        friend class EventDispatcher;
    };

    class EventDispatcher {
        template<typename T>
        using EventFunction = std::function<bool(T&)>;
    public:
        EventDispatcher(Event& event) 
            : m_Event(event) {}

        template<typename T>
        bool Dispatch(EventFunction<T> func) {
            if (!m_Event.Handled && m_Event.GetType() == T::GetStaticType()) {
                m_Event.Handled = func(*(T*)&m_Event);
                return true;
            }
            return false;
        }
    private:
        Event& m_Event;
    };
}
