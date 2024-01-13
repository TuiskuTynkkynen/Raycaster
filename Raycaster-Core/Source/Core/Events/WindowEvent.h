#pragma once

#include "Event.h"

namespace Core {
	class WindowClose : public Event {
	public:
		WindowClose(){}

		static EventType GetStaticType() { return EventType::WindowClose; }
		EventType GetType() const override { return GetStaticType(); }
		int GetCategory() const override { return EventCategoryWindow; }

	};

	class WindowResize : public Event
	{
	private:
		int m_Width;
		int m_Height;
	
	public:
		WindowResize(int width, int heigth)
			: m_Width(width), m_Height(heigth) {}

		inline int GetWidth() const { return m_Width; }
		inline int GetHeigth() const { return m_Height; }

		static EventType GetStaticType() { return EventType::WindowResize; }
		EventType GetType() const override { return GetStaticType(); }
		int GetCategory() const override { return EventCategoryWindow; }
	};

	class WindowFocus : public Event {
	public:
		WindowFocus() {}

		static EventType GetStaticType() { return EventType::WindowFocus; }
		EventType GetType() const override { return GetStaticType(); }
		int GetCategory() const override { return EventCategoryWindow; }
	};

	class WindowLostFocus : public Event {
	public:
		WindowLostFocus() {}

		static EventType GetStaticType() { return EventType::WindowLostFocus; }
		EventType GetType() const override { return GetStaticType(); }
		int GetCategory() const override { return EventCategoryWindow; }
	};
}