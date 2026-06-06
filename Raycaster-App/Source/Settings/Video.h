#pragma once

#include "SavedState.h"

#include "Core/Base/Window.h"
#include "Core/Events/WindowEvent.h"
#include "Core/Serialization/Archive.h"

namespace Settings::Video {
    struct WindowModeCallback {
        void operator()(Core::WindowMode v);
    };
    struct VSyncCallback {
        void operator()(bool v);
    };

    inline SavedState<Core::WindowMode, WindowModeCallback> WindowMode = Core::WindowMode::Windowed;
    inline SavedState<bool, VSyncCallback> VSync = false;

    inline SavedState<bool> LayerRaycaster = true;
    inline SavedState<bool> Layer3D = true;
    inline SavedState<bool> Layer2D = false;

    enum class LayerType : uint8_t {
        Raycaster = 0,
        Layer3D,
        Layer2D,
    };
    bool LayerEnabled(LayerType layer);
    glm::uvec2 ViewPortSize(LayerType layer);
    uint32_t ViewPortOffset(LayerType layer);
    
    bool Serialize(Core::Serialization::Archive& archive);
    bool Deserialize(Core::Serialization::Archive& archive);
}