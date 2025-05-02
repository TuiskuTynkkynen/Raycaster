#include "Effects.h"

#include "Internal.h"
#include "Core/Debug/Debug.h"

namespace Core::Audio::Effects {
    Internal::FilterNode CreateFilterNode(auto settings, ma_node* parent);

    Internal::FilterNode CreateFilterNode(auto settings, ma_node* parent) {
        static_assert(false, "Unsupported type passed as settings");
    }

    static bool ReinitFilterNode(auto* node, ma_node* parent);

    static bool ReinitFilterNode(auto* node, ma_node* parent) {
        static_assert(false, "Pointer to unsupported type passed as node");
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Filter                                                                                                                            //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template <typename T>
    Filter::Filter(T settings, Bus& parent) : m_Child(nullptr), m_Parent(&parent) {
        m_InternalFilter = CreateFilterNode(settings, parent.m_InternalBus.get());
    }

    template <typename T>
    Filter::Filter(T settings, Filter& parent) : m_Child(nullptr), m_Parent(&parent) {
        ma_node* parentNode = std::visit([](auto parent) { return (ma_node*)parent; }, parent.m_InternalFilter);
        m_InternalFilter = CreateFilterNode(settings, parentNode);
    }

    Filter::Filter(Filter&& other) noexcept {
        m_InternalFilter.swap(other.m_InternalFilter);
        m_Child.swap(other.m_Child);

        SwitchParent(other.m_Parent);
        other.SwitchParent(nullptr);
    }

    Filter::~Filter() {
        SwitchParent(nullptr);

        // Only cases where child is another filter need to be considered
        if (std::holds_alternative<Filter*>(m_Child)) {
            std::get<Filter*>(m_Child)->SwitchParent(nullptr);
        }

        std::visit([](auto node) { delete node; }, m_InternalFilter);
    }

    bool Filter::Reinit() {
        ma_node* parentNode = nullptr;

        if (std::holds_alternative<Filter*>(m_Parent)) {
            Filter* parent = std::get<Filter*>(m_Parent);

            if (parent) {
                parentNode = std::visit([](auto parent) { return (ma_node*)parent; }, parent->m_InternalFilter);
            }
        } else if (std::holds_alternative<Bus*>(m_Parent)) {
            Bus* parent = std::get<Bus*>(m_Parent);

            if (parent) {
                parentNode = parent->m_InternalBus.get();
            }
        }

        if (!parentNode) {
            RC_INFO("Reinitializing filter with no parent");
        }

        return std::visit([parentNode, this](auto node) { return ReinitFilterNode(node, parentNode);}, m_InternalFilter);
    }

    void Filter::AttachParent(Bus& parent) {
        if (std::holds_alternative<Bus*>(m_Parent) && std::get<Bus*>(m_Parent) == &parent) {
            return;
        }

        SwitchParent(&parent);

        ma_node* currentNode = std::visit([](auto node) { return (ma_node*)node; }, m_InternalFilter);

        // Currently implemented filter nodes and buses (ma_sound_group) have only one input and output bus, so index is always 0
        ma_result result = ma_node_attach_output_bus(currentNode, 0, parent.m_InternalBus.get(), 0);

        if (result != MA_SUCCESS) {
            RC_WARN("Attaching parent bus to filter failed with error {}", (int32_t)result);
        }
    }

    void Filter::AttachParent(Filter& parent) {
        if (std::holds_alternative<Filter*>(m_Parent) && std::get<Filter*>(m_Parent) == &parent) {
            return;
        }

        SwitchParent(&parent);

        ma_node* currentNode = std::visit([](auto node) { return (ma_node*)node; }, m_InternalFilter);
        ma_node* parentNode = std::visit([](auto parent) { return (ma_node*)parent; }, parent.m_InternalFilter);

        // Currently implemented filter nodes have only one input and output bus, so index is always 0
        ma_result result = ma_node_attach_output_bus(currentNode, 0, parentNode, 0);

        if (result != MA_SUCCESS) {
            RC_WARN("Attaching parent filter to filter failed with error {}", (int32_t)result);
        }
    }

    void Filter::SwitchParent(RelativeNode parent) {
        // Only cases where parent is another filter need to be considered
        if (std::holds_alternative<Filter*>(m_Parent)) {
            std::get<Filter*>(m_Parent)->DetachChild();
        }

        // Only cases where parent is another filter need to be considered
        if (std::holds_alternative<Filter*>(m_Parent)) {
            std::get<Filter*>(m_Parent)->AttachChild(this);
        }

        m_Parent = parent;
    }

    void Filter::AttachChild(RelativeNode child) {
        m_Child = child;
    }

    void Filter::DetachChild() {
        m_Child = nullptr;
    }
}