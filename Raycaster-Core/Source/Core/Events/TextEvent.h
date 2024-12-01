#pragma once

#include "Event.h"

namespace Core {
	class TextInput : public Event
	{
	private: 
		uint32_t m_Character;
	
	public:
		TextInput(uint32_t character) 
			: m_Character(character) {}

		inline uint32_t GetCharacter() const { return m_Character; }

		static EventType GetStaticType() { return EventType::TextInput; }
		EventType GetType() const override { return GetStaticType(); }
		int32_t GetCategory() const override { return EventCategoryInput | EventCategoryText; }
	};
}