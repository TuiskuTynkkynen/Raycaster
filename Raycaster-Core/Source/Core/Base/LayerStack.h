#pragma once

#include "Layer.h"

#include <vector>
#include <span>
#include <memory>

namespace Core {
    class LayerStack {
    public:
        LayerStack() = default;
        ~LayerStack();

        LayerStack(const LayerStack&) = delete;
        LayerStack& operator=(const LayerStack&) = delete;
        LayerStack(LayerStack&&) noexcept = default;
        LayerStack& operator=(LayerStack&&) noexcept = default;

        void PushLayer(std::unique_ptr<Layer> layer, std::weak_ptr<Scene> active);
        void PushOverlay(std::unique_ptr<Layer> overlay, std::weak_ptr<Scene> active);
        bool PopLayer(Layer* layer);
        bool PopOverlay(Layer* overlay);

        std::span<Layer*> Layers() { return m_Layers; }
        inline size_t Size() const { return m_Layers.size(); }
        void Clear();
    private:
        std::vector<Layer*> m_Layers;
        size_t m_OverlayOffset = 0;
    };
}