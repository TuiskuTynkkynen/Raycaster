#pragma once

#include "Event.h"

namespace Core {
	class KeyPressed : public Event
	{
	private: 
		int m_KeyCode;
		bool m_Repeated;
	
	public:
		KeyPressed(int keyCode, bool repeated) 
			: m_KeyCode(keyCode), m_Repeated(repeated) {}

		inline int GetKeyCode() const { return m_KeyCode; }
		inline bool IsRepeated() const { return m_Repeated; }

		static EventType GetStaticType() { return EventType::KeyPressed; }
		EventType GetType() const override { return GetStaticType(); }
		int GetCategory() const override { return EventCategoryInput | EventCategoryKeyboard; }
	};
}