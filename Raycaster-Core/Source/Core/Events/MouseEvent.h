#pragma once

#include "Event.h"

#include <glm/glm.hpp>

namespace Core {
	class MouseButtonPressed : public Event {
	public:
		MouseButtonPressed(int32_t button)
			: m_Button(button) {}

		inline int32_t GetButton() const { return m_Button; }

		static EventType GetStaticType() { return EventType::MouseButtonPressed; }
		EventType GetType() const override { return GetStaticType(); }
		int32_t GetCategory() const override { return EventCategoryInput | EventCategoryMouse; }
	private:
		int32_t m_Button;
	};
	
	class MouseButtonReleased : public Event {
	public:
		MouseButtonReleased(int32_t button)
			: m_Button(button) {}

		inline int32_t GetButton() const { return m_Button; }

		static EventType GetStaticType() { return EventType::MouseButtonReleased; }
		EventType GetType() const override { return GetStaticType(); }
		int32_t GetCategory() const override { return EventCategoryInput | EventCategoryMouse; }
	private:
		int32_t m_Button;
	};


	class MouseMoved : public Event {
	public:
		MouseMoved(float x, float y)
			: m_Position(x, y) {}

		inline glm::vec2 GetPosition() const { return m_Position; }
		inline float GetPositionX() const { return m_Position.x; }
		inline float GetPositionY() const { return m_Position.y; }

		static EventType GetStaticType() { return EventType::MouseMoved; }
		EventType GetType() const override { return GetStaticType(); }
		int32_t GetCategory() const override { return EventCategoryInput | EventCategoryMouse; }
	private:
		glm::vec2 m_Position;
	};


	class MouseScrolled : public Event {
	public:
		MouseScrolled(float x, float y)
			: m_Offset(x, y) {}

		inline glm::vec2 GetOffset() const { return m_Offset; }
		inline float GetOffsetX() const { return m_Offset.x; }
		inline float GetOffsetY() const { return m_Offset.y; }

		static EventType GetStaticType() { return EventType::MouseScrolled; }
		EventType GetType() const override { return GetStaticType(); }
		int32_t GetCategory() const override { return EventCategoryInput | EventCategoryMouse; }
	private:
		glm::vec2 m_Offset;
	};
}