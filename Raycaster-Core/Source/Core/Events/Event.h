#pragma once

#include <functional>

#define Bit(x) (1 << x)

namespace Core {
	enum class EventType {
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus,
		KeyPressed,
		MouseButtonPressed, MouseMoved, MouseScrolled 
	};

	enum EventCategory {
		None = 0,
		EventCategoryWindow      = Bit(0),
		EventCategoryInput       = Bit(1),
		EventCategoryKeyboard    = Bit(2),
		EventCategoryMouse	     = Bit(3),
	};

	class Event
	{
		friend class EventDispatcher;

	public:
		virtual EventType GetType() const = 0;
		virtual int GetCategory() const = 0;

		inline bool IsInCategory(EventCategory category) const {
			return GetCategory() & category;
		}
	
	protected:
		bool m_Handled = false;
	};

	class EventDispatcher {
		template<typename T>
		using EventFunction = std::function<bool(T&)>;
	
	private:
		Event& m_Event;
	
	public:
		EventDispatcher(Event& event) 
			: m_Event(event) {}

		template<typename T>
		bool Dispatch(EventFunction<T> func) {
			if (m_Event.GetType() == T::GetStaticType()) {
				m_Event.m_Handled = func(*(T*)&m_Event);
				return true;
			}
			return false;
		}
	};
}