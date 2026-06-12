#pragma once

#include "Layer.h"

#include <vector>
#include <span>
#include <memory>
#include <typeinfo>

namespace Core {
    class LayerCache {
    public:
        void Append(class LayerStack&& previous);
        std::unique_ptr<Layer> TryPop(const std::type_info& type);
    private:
        std::vector<std::unique_ptr<Layer>> m_Cached;
    };

    class LayerStack {
    public:
        LayerStack() = default;
        ~LayerStack();

        LayerStack(const LayerStack&) = delete;
        LayerStack& operator=(const LayerStack&) = delete;
        LayerStack(LayerStack&&) noexcept = default;
        LayerStack& operator=(LayerStack&&) noexcept = default;

        bool PushCachedLayer(const std::type_info& layerType, LayerCache& cache, std::weak_ptr<Scene> active);
        bool PushCachedOverlay(const std::type_info& overlayType, LayerCache& cache, std::weak_ptr<Scene> active);

        void PushLayer(std::unique_ptr<Layer> layer, std::weak_ptr<Scene> active);
        void PushOverlay(std::unique_ptr<Layer> overlay, std::weak_ptr<Scene> active);
        bool PopLayer(Layer* layer);
        bool PopOverlay(Layer* overlay);

        std::span<Layer*> Layers() { return m_Layers; }
        inline size_t Size() const { return m_Layers.size(); }
        void Clear();
    private:
        friend void LayerCache::Append(LayerStack&&);
        std::vector<Layer*> m_Layers;
        size_t m_OverlayOffset = 0;
    };
}