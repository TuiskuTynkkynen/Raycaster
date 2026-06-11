#include "LayerStack.h"

#include "Core/Debug/Assert.h"

#include <algorithm>
#include <ranges>

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
}
