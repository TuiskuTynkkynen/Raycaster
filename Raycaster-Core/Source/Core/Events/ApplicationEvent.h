#pragma once

#include "Event.h"

namespace Core {
	class ApplicationClose : public Event {
	public:
		ApplicationClose() {}

		static EventType GetStaticType() { return EventType::ApplicationClose; }
		EventType GetType() const override { return GetStaticType(); }
		int GetCategory() const override { return EventCategoryApplication; }
	};
}