#include "LayerStack.h"

#include "Core/Debug/Assert.h"

#include <algorithm>
#include <ranges>
#include <utility>

namespace Core {
    LayerStack::~LayerStack() {
        Clear();
    }

    void LayerStack::Clear() {
        for (Layer* layer : m_Layers | std::ranges::views::reverse) {
            RC_ASSERT(layer != nullptr);
            layer->OnDetach();
            delete layer;
        }

        m_Layers.clear();
        m_OverlayOffset = 0;
    }

    bool LayerStack::PushCachedLayer(const std::type_info& layerType, LayerCache& cache, std::weak_ptr<Scene> active) {
        if (auto layer = cache.TryPop(layerType)) {
            PushLayer(std::move(layer), active);
            return true;
        }

        return false;
    }

    bool LayerStack::PushCachedOverlay(const std::type_info& overlayType, LayerCache& cache, std::weak_ptr<Scene> active) {
        if (auto overlay = cache.TryPop(overlayType)) {
            PushOverlay(std::move(overlay), active);
            return true;
        }
        
        return false;
    }

    void LayerStack::PushLayer(std::unique_ptr<Layer> layer, std::weak_ptr<Scene> active) {
        RC_ASSERT(m_OverlayOffset <= m_Layers.size());
        RC_ASSERT(layer);
        layer->SetScene(active);
        layer->OnAttach();

        m_Layers.emplace(m_Layers.begin() + m_OverlayOffset, layer.release());
        m_OverlayOffset++;
    }

    void LayerStack::PushOverlay(std::unique_ptr<Layer> overlay, std::weak_ptr<Scene> active) {
        RC_ASSERT(overlay);
        overlay->SetScene(active);
        overlay->OnAttach();

        m_Layers.emplace_back(overlay.release());
    }

    bool LayerStack::PopLayer(Layer* layer) {
        auto iter = std::find(m_Layers.begin(), m_Layers.end(), layer);
        if (iter == m_Layers.end() && *iter == nullptr) { return false; }

        (*iter)->OnDetach();
        m_Layers.erase(iter);
        m_OverlayOffset--;

        return true;
    }

    bool LayerStack::PopOverlay(Layer* overlay) {
        auto iter = std::find(m_Layers.begin(), m_Layers.end(), overlay);
        if (iter == m_Layers.end() && *iter == nullptr) { return false; }

        (*iter)->OnDetach();
        m_Layers.erase(iter);

        return true;
    }
    
    void LayerCache::Append(LayerStack&& previous) {
        m_Cached.reserve(m_Cached.size() + previous.m_Layers.size());

        for (Layer* layer : previous.m_Layers | std::ranges::views::reverse) {
            RC_ASSERT(layer != nullptr);
            layer->OnDetach();
            m_Cached.emplace_back(layer);
        }

        previous.m_Layers.clear();
    }

    std::unique_ptr<Layer> LayerCache::TryPop(const std::type_info& type) {
        for (size_t i = 0; i < m_Cached.size(); i++) {
            RC_ASSERT(m_Cached[i]);
            Layer& layer = *m_Cached[i];
            if (type != typeid(layer)) { continue; }
           
            // Swap and pop
            std::unique_ptr result = std::exchange(m_Cached[i], std::move(m_Cached.back()));
            m_Cached.pop_back();
            return result;
        }

        return std::unique_ptr<Layer>{ nullptr };
    }
}
