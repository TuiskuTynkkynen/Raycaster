#pragma once

#include <glm/glm.hpp>

#include <array>
#include <memory>

namespace Core::UI {
    namespace {
        std::array<glm::vec4, 3> DefaultColours = {
            glm::vec4(0.1f, 0.1f, 0.125f, 1.0f),
            glm::vec4(0.15f, 0.15f, 0.175f, 1.0f),
            glm::vec4(0.2f, 0.2f, 0.25f, 1.0f),
        };

        std::array<glm::vec4, 3> DefaultTextColours = {
            glm::vec4(0.8f, 0.8f, 0.8f, 1.0f),
            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
            glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)
        };
        

        std::array<glm::vec4, 3> DefaultInputTextColours = {
            glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
            glm::vec4(0.8f, 0.8f, 0.8f, 1.0f),
            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
        };

        std::array<glm::vec4, 3> DefaultTextureColours = {
            glm::vec4(1.0f),
            glm::vec4(1.0f),
            glm::vec4(1.0f)
        };

        std::array<glm::vec4, 3> DefaultHighlightColours = {
            glm::vec4(0.2f, 0.25f, 0.4f, 1.0f),
            glm::vec4(0.2f, 0.35f, 0.6f, 1.0f),
            glm::vec4(0.3f, 0.3f, 0.7f, 1.0f),
        };

        std::array<glm::vec4, 3> DefaultBackgroundColours = {
            glm::vec4(0.05f, 0.05f, 0.06f, 1.0f),
            glm::vec4(0.05f, 0.05f, 0.06f, 1.0f),
            glm::vec4(0.05f, 0.05f, 0.06f, 1.0f),
        };
    }

    enum class LayoutType {
        None = 0,
        Vertical,
        Horizontal,
        Crop,
        CropVertical,
        CropHorizontal,
    };
    
    enum class SurfaceType {
        None = 0,
        Hoverable,
        Activatable,
        Button,
        Capture,
        TextInput,
    };

    enum class PositioningType {
        Auto = 0,
        Offset,
        Relative,
        Absolute,
    };

    enum class HoverResult : uint8_t {
        None = 0,
        Hovered,
        Active,
    };

    enum class TextAlignment : uint8_t {
        Left = 0,
        Right,
    };

    class Widget;

    struct Surface {
        SurfaceType Type;
        LayoutType Layout;
        PositioningType Positioning;

        glm::vec2 Position;
        glm::vec2 Size;

        std::array<glm::vec4, 3> Colours; // [0] = primary, [1] = hover, [2] = clicked

        size_t ParentID = 0;
        size_t SiblingID = 0;
        uint32_t ChildCount = 0;

        std::unique_ptr<Widget> Widget;

        Surface(SurfaceType type = SurfaceType::None, LayoutType layout = LayoutType::None, PositioningType positioning = PositioningType::Auto,
                glm::vec2 position = glm::vec2(1.0f), glm::vec2 size = glm::vec2(1.0f),
                std::array<glm::vec4, 3> colours = DefaultColours, size_t parentID = 0); 
    };

    class Widget {
    public:
        virtual ~Widget(){};

        virtual void Update(Surface& current) = 0;
        virtual bool Render(Surface& current) = 0;
    };

    inline Surface::Surface(SurfaceType type, LayoutType layout, PositioningType positioning, glm::vec2 position, glm::vec2 size, 
            std::array<glm::vec4, 3> colours, size_t parentID) 
        : Type(type), Layout(layout), Positioning(positioning), Position(position), Size(size), Colours(colours), ParentID(parentID) {}

    struct AtlasProperties {
        glm::uvec3 Indices{};
        glm::vec2 Size{};
    };

    struct InputAtlasProperties {
        const glm::uvec3 BoxAtlasIndices{};
        const glm::vec2 BoxSize{};

        const uint32_t CaretIndex{};
        const glm::vec2 CaretSize{};

        const uint32_t SelectionMiddleIndex{};
        const glm::vec2 SelectionMiddleSize{};

        const uint32_t SelectionLeftEndIndex{};
        const uint32_t SelectionRightEndIndex{};
        const glm::vec2 SelectionEndsSize{};
    };
}
