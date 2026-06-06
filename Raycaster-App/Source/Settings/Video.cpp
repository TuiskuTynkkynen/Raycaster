#include "Video.h"

#include "Core/Serialization/Serialization.h"
#include "Core/Base/Application.h"

#include <utility>

using WindowMode_t = std::underlying_type_t<Core::WindowMode>;

namespace Settings {
    void Video::WindowModeCallback::operator()(Core::WindowMode v) {
        Core::Application::GetWindow().SetMode(v); 
    }

    void Video::VSyncCallback::operator()(bool v) {
        Core::Application::GetWindow().SetVSync(v); 
    }

    bool Video::LayerEnabled(LayerType layer) {
        switch (layer) {
        case LayerType::Raycaster: return LayerRaycaster;
        case LayerType::Layer3D: return Layer3D;
        case LayerType::Layer2D: return Layer2D;
        }

        RC_ASSERT(false); // Should not be reached
        return false;
    }

    glm::uvec2 Video::ViewPortSize(LayerType layer) {
        if (!LayerEnabled(layer)) { return {}; }

        static const Core::Window& window = Core::Application::GetWindow();
        const float layerCount = static_cast<float>(LayerRaycaster + Layer3D + Layer2D);
        return { glm::ceil(window.GetWidth() / layerCount), window.GetHeight() };
    }

    uint32_t Video::ViewPortOffset(LayerType layer) {
        if (!LayerEnabled(layer)) { return {}; }

        uint32_t offset = 0;
        for (size_t i = 0; i < static_cast<size_t>(layer); i++) {
            offset += ViewPortSize(static_cast<LayerType>(i)).x;
        }
        return offset;
    }

    bool Video::Serialize(Core::Serialization::Archive& archive) {
        static_assert(sizeof(WindowMode_t) == sizeof(bool));
        constexpr size_t count = 5;

        bool success = archive.Write<size_t>(count);
        
        success &= archive.Write(std::to_underlying(Video::WindowMode.GetValue()));
        if (success) Video::WindowMode.SetSavedExternal();

        success &= Video::VSync.Serialize(archive);
        success &= Video::LayerRaycaster.Serialize(archive);
        success &= Video::Layer3D.Serialize(archive);
        success &= Video::Layer2D.Serialize(archive);

        return success;
    }

    bool Video::Deserialize(Core::Serialization::Archive& archive) {
        static_assert(sizeof(WindowMode_t) == sizeof(bool));
        const size_t count = archive.Read<size_t>().value_or(0);
        const size_t position = archive.GetPosition() + count * sizeof(bool);

        if (count < 5) {
            archive.SeekPosition(position);
            return false; 
        }

        auto mode = archive.Read<WindowMode_t>();
        bool success = mode && mode.value() <= std::to_underlying(Core::WindowMode::Fullscreen);
        if (success) {
            Video::WindowMode.Update(static_cast<Core::WindowMode>(mode.value()));
            Video::WindowMode.SetSavedExternal();
        }

        success &= Video::VSync.Deserialize(archive);
        success &= Video::LayerRaycaster.Deserialize(archive);
        success &= Video::Layer3D.Deserialize(archive);
        success &= Video::Layer2D.Deserialize(archive);

        archive.SeekPosition(position);
        return success;
    }
}
