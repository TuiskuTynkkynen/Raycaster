#pragma once

#include "Event.h"
#include "Core/Scene/Scene.h"

#include <memory>
#include <utility>

namespace Core {
    class ApplicationClose : public Event {
    public:
        ApplicationClose() {}

        static EventType GetStaticType() { return EventType::ApplicationClose; }
        EventType GetType() const override { return GetStaticType(); }
        int GetCategory() const override { return EventCategoryApplication; }
    };
    
    class ApplicationScenePush : public Event {
    public:
        ApplicationScenePush(std::shared_ptr<Scene> scene) 
            : m_Scene(scene) {}

        inline std::shared_ptr<Scene> GetScene() { return std::exchange(m_Scene, {}); }

        static EventType GetStaticType() { return EventType::ApplicationScenePush; }
        EventType GetType() const override { return GetStaticType(); }
        int GetCategory() const override { return EventCategoryApplication; }
    private:
        std::shared_ptr<Scene> m_Scene;
    };
    
    class ApplicationScenePop : public Event {
    public:
        static EventType GetStaticType() { return EventType::ApplicationScenePop; }
        EventType GetType() const override { return GetStaticType(); }
        int GetCategory() const override { return EventCategoryApplication; }
    };
}