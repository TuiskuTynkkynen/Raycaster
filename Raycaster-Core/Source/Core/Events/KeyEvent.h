#pragma once

#include "Event.h"

namespace Core {
	class KeyPressed : public Event {
	public:
		KeyPressed(int32_t keyCode, bool repeated) 
			: m_KeyCode(keyCode), m_Repeated(repeated) {}

		inline int32_t GetKeyCode() const { return m_KeyCode; }
		inline bool IsRepeated() const { return m_Repeated; }

		static EventType GetStaticType() { return EventType::KeyPressed; }
		EventType GetType() const override { return GetStaticType(); }
		int32_t GetCategory() const override { return EventCategoryInput | EventCategoryKeyboard; }
	private:
		int32_t m_KeyCode;
		bool m_Repeated;
	};

	class KeyReleased : public Event {
	public:
		KeyReleased(int32_t keyCode) 
			: m_KeyCode(keyCode) {}

		inline int32_t GetKeyCode() const { return m_KeyCode; }

		static EventType GetStaticType() { return EventType::KeyReleased; }
		EventType GetType() const override { return GetStaticType(); }
		int32_t GetCategory() const override { return EventCategoryInput | EventCategoryKeyboard; }
	private:
		int32_t m_KeyCode;
	};
}