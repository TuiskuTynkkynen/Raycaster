#include "BusManager.h"

#include "Sound.h"
#include "Core/Debug/Debug.h"

namespace Core::Audio {
    BusManager::~BusManager() {
        for (size_t i = 0; i < m_BusNames.size(); i++) {
            delete[] m_BusNames[i];
    }
    }

    void BusManager::Init(uint32_t initialCapacity) {
        RC_ASSERT(m_Buses.empty() || !m_Buses.front(), "BusManager has already been initialized");

        m_Buses.reserve(initialCapacity);
        m_BusIndices.reserve(initialCapacity);
        m_BusNames.reserve(initialCapacity);

        m_BusIndices.emplace(StoreName("Master"), Index{ .Epoch = m_Epoch, .Value = 0 });
        m_Buses.emplace_back().emplace();
    }

    void BusManager::Shutdown() {
        for (size_t i = 0; i < m_BusNames.size(); i++) {
            delete[] m_BusNames[i];
        }

        m_Buses.clear();
        m_BusIndices.clear();
        m_BusNames.clear();

        m_Epoch = 0;
        m_IsDense = true;
    }

    void BusManager::RegisterBus(std::string_view name) {
        RegisterBus(name, GetMasterBus());
    }

    void BusManager::RegisterBus(std::string_view name, Bus& parentBus) {
        RC_ASSERT(!m_Buses.empty() && m_Buses.front(), "BusManager has not been initialized");

        if (m_BusIndices.contains(name)) {
            RC_WARN("Tried to register multiple buses with the same name");
            return;
        }

        std::vector<std::optional<Bus>>::iterator iter = m_Buses.end();
        if (m_IsDense || (iter = std::ranges::find_if(m_Buses, [](const std::optional<Bus>& item) { return !item; })) == m_Buses.end()) {
            m_IsDense = true;

            RC_ASSERT(m_Buses.size() <= std::numeric_limits<uint32_t>::max(), "Audio System supports only up to UINT32_MAX concurrent registered buses");
            uint32_t index = static_cast<uint32_t>(m_Buses.size());

            m_BusIndices.emplace(StoreName(name), Index{ .Epoch = m_Epoch, .Value = index });

            m_Buses.emplace_back().emplace(parentBus);
            return;
        }

        iter->emplace(parentBus);

        RC_ASSERT(iter - m_Buses.begin() <= std::numeric_limits<uint32_t>::max(), "Audio System supports only up to UINT32_MAX concurrent registered buses");
        uint32_t index = static_cast<int32_t>(iter - m_Buses.begin());

        m_BusIndices.emplace(StoreName(name, index), Index{ .Epoch = m_Epoch, .Value = index });

        m_IsDense = std::ranges::find_if(++iter, m_Buses.end(), [](const std::optional<Bus>& item) { return !item; }) == m_Buses.end();
    }

    void BusManager::RegisterBus(std::string_view name, Index parentBusIndex) {
        if (!IndexIsValid(parentBusIndex) || !m_Buses[parentBusIndex.Value].has_value()) {
            RC_WARN("Tried to register bus with invalid parent bus index");
            return;
        }

        RegisterBus(name, m_Buses[parentBusIndex.Value].value());
    }

    void BusManager::RegisterBus(std::string_view name, std::string_view parentBusName) {
        Bus* parent = GetBus(parentBusName);
        if (!parent) {
            RC_WARN("Tried to register bus with invalid parent bus name");
            return;
        }

        RegisterBus(name, *parent);
    }


    void BusManager::UnregisterBus(std::string_view name) {
        Index index = GetBusIndex(name);

        if (index.Value == 0) {
            RC_WARN("Can not unregister master bus");
            return;
        }

        m_BusIndices.erase(name);

        if (IndexIsValid(index)) {
            m_IsDense = index.Value == (m_Buses.size() - 1); // If last element, m_Buses is still dense

            auto& bus = m_Buses[index.Value];
            if (bus) {
                Bus* parent = bus.value().m_Parent;

                // Move all children to parent
                for (auto& variant : bus.value().m_Children) {
                    std::visit([parent](auto& child) { child->SwitchParent(parent); }, variant);
                }

                bus.reset();
            }

            delete[] m_BusNames[index.Value];
            m_BusNames[index.Value] = nullptr;
        }
    }

    void BusManager::Compact() {
        if (m_IsDense) {
            return;
        }

        RC_ASSERT(m_Buses.size() == m_BusNames.size(), "Size of buses and bus names vectors should always be equal");
        struct Moved {
            uint32_t From;
            uint32_t To;
        };
        std::vector<Moved> moved;

        // First pass move all nullopts to back
        RC_ASSERT(m_Buses.size() <= std::numeric_limits<uint32_t>::max(), "Audio System supports only up to UINT32_MAX concurrent registered buses");
        for (size_t i = 0; i < m_Buses.size(); i++) {
            if (m_Buses[i]) {
                continue;
            }

            uint32_t oldIndex = static_cast<uint32_t>(m_Buses.size() - 1);

            moved.emplace_back(static_cast<uint32_t>(oldIndex), static_cast<uint32_t>(i)); // oldIndex and i can never be larger than UINT32_MAX

            auto& back = m_Buses.back();
            if (back) {
                m_Buses[i].emplace(std::move(back.value()));
            }
            m_Buses.pop_back();

            m_BusNames[i] = m_BusNames.back();
            m_BusNames.pop_back();
        }

        // Second pass remove all nullopts from back
        for (size_t i = m_Buses.size(); i > 0;) {
            if (m_Buses[--i]) {
                break;
            }

            m_Buses.pop_back();
            m_BusNames.pop_back();
        }

        m_IsDense = true;

        if (moved.empty()) {
            return;
        }

        m_Epoch++;
        for (auto& [name, index] : m_BusIndices) {
            for (size_t i = 0; i < moved.size(); i++) {
                if (moved[i].From == index.Value) {
                    index.Value = moved[i].To;

                    moved[i] = moved.back();
                    moved.pop_back();
                    break;
                }
            }

            index.Epoch = m_Epoch;
        }
    }

    void BusManager::ReinitBuses() {
        for (auto& bus : m_Buses) {
            if (!bus) {
                continue;
            }

            bus.value().Reinit();
        }
    }

    bool BusManager::IndexIsValid(Index index) {
        return index.Epoch == m_Epoch && index.Value < m_Buses.size();
    }

    Index BusManager::ValidateIndex(Index index, std::string_view name) {
        // Already invalidated or valid index
        if (!index || IndexIsValid(index)) {
            return index;
        }

        return GetBusIndex(name);
    }

    Index BusManager::GetBusIndex(std::string_view name) {
        if (auto iter = m_BusIndices.find(name); iter != m_BusIndices.end()) {
            return iter->second;
        }

        return  Index{}; // Return invalidated index
    }

    Bus& BusManager::GetMasterBus() {
        RC_ASSERT(!m_Buses.empty() && m_Buses.front(), "m_Buses should always have the master bus as it's 1st element");
        return m_Buses.front().value();
    }

    Index BusManager::GetMasterBusIndex() {
        RC_ASSERT(!m_Buses.empty() && m_Buses.front(), "m_Buses should always have the master bus as it's 1st element");

        return { .Epoch = m_Epoch, .Value = 0 };
    }

    std::string_view BusManager::GetMasterBusName() {
        RC_ASSERT(!m_Buses.empty() && m_Buses.front(), "m_Buses should always have the master bus as it's 1st element");

        return "Master";
    }

    Bus* BusManager::GetBus(Index index) {
        if (IndexIsValid(index) && m_Buses[index.Value]) {
            return &m_Buses[index.Value].value();
        }

        return nullptr;
    }

    Bus* BusManager::GetBus(std::string_view name) {
        return GetBus(GetBusIndex(name));
    }

    Bus* BusManager::GetParent(ChildNode node) {
        return std::visit([](auto& child) { if (!child) { return (Bus*)nullptr; } return child->m_Parent; }, node);
    }

    const std::vector<ChildNode>& BusManager::GetChildren(Bus& bus) {
        return bus.m_Children;
    }

    bool BusManager::IsParent(const Bus* parent, ChildNode child) {
        return std::visit([parent](auto& node) { return node && node->m_Parent == parent; }, child);
    }

    bool BusManager::IsParent(Index parentIndex, ChildNode child) {
        Bus* bus = GetBus(parentIndex);

        if (!bus) {
            RC_WARN("Could not find bus, with index = {}, and epoch = {}", parentIndex.Value, parentIndex.Epoch);
            return false;
        }

        return IsParent(bus, child);
    }

    bool BusManager::IsParent(std::string_view parentName, ChildNode child) {
        Bus* bus = GetBus(parentName);

        if (!bus) {
            RC_WARN("Could not find bus \"{}\"", parentName);
            return false;
        }

        return IsParent(bus, child);
    }

    bool BusManager::IsAncestor(const Bus* ancestor, ChildNode child) {
        if (ancestor == nullptr) {
            return false;
        }

        Bus* current = std::visit([ancestor](auto& node) { if (!node) { return (Bus*)nullptr; } return node->m_Parent; }, child);

        while (current) {
            if (current == ancestor) {
                return true;
            }

            current = current->m_Parent;
        }

        return false;
    }

    bool BusManager::IsAncestor(Index ancestorIndex, ChildNode child) {
        Bus* bus = GetBus(ancestorIndex);

        if (!bus) {
            RC_WARN("Could not find bus, with index = {}, and epoch = {}", ancestorIndex.Value, ancestorIndex.Epoch);
            return false;
        }

        return IsAncestor(bus, child);
    }

    bool BusManager::IsAncestor(std::string_view ancestorName, ChildNode child) {
        Bus* bus = GetBus(ancestorName);

        if (!bus) {
            RC_WARN("Could not find bus \"{}\"", ancestorName);
            return false;
        }

        return IsAncestor(bus, child);
    }

    size_t BusManager::BusCount() {
        return m_BusIndices.size();
    }

    std::string_view BusManager::StoreName(std::string_view name) {
        char* cString = new char[name.size()];
        name.copy(cString, name.size(), (size_t)0);

        m_BusNames.emplace_back(cString);

        return std::string_view(cString, name.size());
    }

    std::string_view BusManager::StoreName(std::string_view name, uint32_t index) {
        char* cString = new char[name.size()];
        name.copy(cString, name.size(), (size_t)0);

        m_BusNames[index] = cString;

        return std::string_view(cString, name.size());
    }
}