#pragma once

#include "Types.h"

#include <memory>

namespace Core::UI {
    class Widget;

    namespace Internal {
        struct WidgetDeleter {
            void operator()(Widget* w);
        };
    }

    enum class SurfaceType : uint8_t {
        None = 0,
        Hoverable,
        Activatable,
        Button,
        Capture,
        TextInput,
    };

    struct Surface {
        SurfaceType Type;
        LayoutType Layout;
        PositioningType Positioning;

        glm::vec3 Position;
        glm::vec2 Size;

        std::array<glm::vec4, 3> Colours; // [0] = primary, [1] = hover, [2] = clicked

        float ChildGap;
        uint32_t ChildCount = 0;
        size_t ParentID = 0;
        size_t SiblingID = 0;

        std::unique_ptr<Widget, Internal::WidgetDeleter> Widget;

        Surface(SurfaceType type = SurfaceType::None, LayoutType layout = LayoutType::None, PositioningType positioning = PositioningType::Auto,
            glm::vec3 position = glm::vec3(0.0f), glm::vec2 size = glm::vec2(1.0f), std::array<glm::vec4, 3> colours = DefaultColours,
            size_t parentID = 0, float childGap = Uninitialized<float>());

        Surface(SurfaceType type = SurfaceType::None, LayoutType layout = LayoutType::None, PositioningType positioning = PositioningType::Auto,
            glm::vec2 position = glm::vec2(0.0f), glm::vec2 size = glm::vec2(1.0f), std::array<glm::vec4, 3> colours = DefaultColours,
            size_t parentID = 0, float childGap = Uninitialized<float>());
    };

    class Widget {
    public:
        virtual ~Widget() {};

        virtual void Update(Surface& current) = 0;
        virtual bool Render(Surface& current) = 0;
    };

    inline Surface::Surface(SurfaceType type, LayoutType layout, PositioningType positioning, glm::vec3 position, glm::vec2 size,
        std::array<glm::vec4, 3> colours, size_t parentID, float childGap)
        : Type(type), Layout(layout), Positioning(positioning), Position(position), Size(size), Colours(colours), ParentID(parentID), ChildGap(childGap) {
    }

    inline Surface::Surface(SurfaceType type, LayoutType layout, PositioningType positioning, glm::vec2 position, glm::vec2 size,
        std::array<glm::vec4, 3> colours, size_t parentID, float childGap)
        : Type(type), Layout(layout), Positioning(positioning), Position(glm::vec3(position, 0.0f)), Size(size), Colours(colours), ParentID(parentID), ChildGap(childGap) {
    }
}
