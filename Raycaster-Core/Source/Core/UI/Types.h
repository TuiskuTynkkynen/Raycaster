#pragma once

#include <glm/glm.hpp>

#include <array>
#include <limits>

namespace Core::UI {
    namespace {
        inline std::array<glm::vec4, 3> DefaultColours = {
            glm::vec4(0.1f, 0.1f, 0.125f, 1.0f),
            glm::vec4(0.15f, 0.15f, 0.175f, 1.0f),
            glm::vec4(0.2f, 0.2f, 0.25f, 1.0f),
        };

        inline std::array<glm::vec4, 3> DefaultTextColours = {
            glm::vec4(0.8f, 0.8f, 0.8f, 1.0f),
            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
            glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)
        };
        
        inline std::array<glm::vec4, 3> DefaultInputTextColours = {
            glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
            glm::vec4(0.8f, 0.8f, 0.8f, 1.0f),
            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
        };

        inline std::array<glm::vec4, 3> DefaultTextureColours = {
            glm::vec4(1.0f),
            glm::vec4(1.0f),
            glm::vec4(1.0f)
        };

        inline std::array<glm::vec4, 3> DefaultHighlightColours = {
            glm::vec4(0.2f, 0.25f, 0.4f, 1.0f),
            glm::vec4(0.2f, 0.35f, 0.6f, 1.0f),
            glm::vec4(0.3f, 0.3f, 0.7f, 1.0f),
        };

        inline std::array<glm::vec4, 3> DefaultBackgroundColours = {
            glm::vec4(0.05f, 0.05f, 0.06f, 1.0f),
            glm::vec4(0.05f, 0.05f, 0.06f, 1.0f),
            glm::vec4(0.05f, 0.05f, 0.06f, 1.0f),
        };

        inline constexpr std::array<glm::vec4, 3> Transparent = {
            glm::vec4(0.0f),
            glm::vec4(0.0f),
            glm::vec4(0.0f)
        };

        template<typename T>
        inline consteval T Uninitialized() requires std::is_same_v<T, float> {
            return std::numeric_limits<float>::quiet_NaN();
        }

        template<typename T>
        inline constexpr bool IsUninitialized(T value) requires std::is_same_v<T, float> {
            return glm::isnan(value);
        }
    }

    enum class LayoutType : uint8_t {
        None = 0,
        Vertical,
        Horizontal,
        Crop,
        CropVertical,
        CropHorizontal,
    };

    enum class PositioningType : uint8_t {
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
