#pragma once

#include "Event.h"

namespace Core {
	class MouseButtonPressed : Event
	{
	private:
		int m_Button;
	
	public:
		MouseButtonPressed(int button)
			: m_Button(button) {}

		inline int GetButton() const { return m_Button; }

		static EventType GetStaticType() { return EventType::MouseButtonPressed; }
		EventType GetType() const override { return GetStaticType(); }
		int GetCategory() const override { return EventCategoryInput | EventCategoryMouse; }
	};


	class MouseMoved: Event
	{
	private:
		float m_MouseX;
		float m_MouseY;
	
	public:
		MouseMoved(float x, float y)
			: m_MouseX(x), m_MouseY(y) {}

		inline int GetPositionX() const { return m_MouseX; }
		inline int GetPositionY() const { return m_MouseY; }

		static EventType GetStaticType() { return EventType::MouseMoved; }
		EventType GetType() const override { return GetStaticType(); }
		int GetCategory() const override { return EventCategoryInput | EventCategoryMouse; }
	};


	class MouseScrolled: Event
	{
	private:
		float m_OffsetX;
		float m_OffsetY;
	
	public:
		MouseScrolled(float x, float y)
			: m_OffsetX(x), m_OffsetY(y) {}

		inline int GetOffsetX() const { return m_OffsetX; }
		inline int GetOffsetY() const { return m_OffsetY; }

		static EventType GetStaticType() { return EventType::MouseScrolled; }
		EventType GetType() const override { return GetStaticType(); }
		int GetCategory() const override { return EventCategoryInput | EventCategoryMouse; }
	};
}