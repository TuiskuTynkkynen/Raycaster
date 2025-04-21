#pragma once

#include "Types.h"
#include "Bus.h"

#include <unordered_map>

namespace Core::Audio {
    class BusManager {
    public:
        BusManager() = default;
        ~BusManager();

        BusManager(const BusManager& other) = delete;
        BusManager(BusManager&& other) noexcept = delete;
        BusManager& operator = (const BusManager& other) = delete;
        BusManager& operator = (const BusManager&& other) = delete;

        void Init(uint32_t initialCapacity = 1);
        void Shutdown();

        void RegisterBus(std::string_view name); // Automatically attaches the master bus as the parent
        void RegisterBus(std::string_view name, Bus& parentBus);
        void RegisterBus(std::string_view name, Index parentBusIndex);
        void RegisterBus(std::string_view name, std::string_view parentBusName);

        void UnregisterBus(std::string_view Name);

        void Compact(); // Calling compact invalidates stored indices

        void ReinitBuses();

        bool IndexIsValid(Index index);
        Index ValidateIndex(Index index, std::string_view name);
        Index GetBusIndex(std::string_view name);

        Bus& GetMasterBus();
        Index GetMasterBusIndex();
        std::string_view GetMasterBusName();

        Bus* GetBus(Index index);
        Bus* GetBus(std::string_view name);

        Bus* GetParent(ChildNode node);
        const std::vector<ChildNode>& GetChildren(Bus& bus);

        bool IsParent(const Bus* parent, ChildNode child);
        bool IsParent(Index parentIndex, ChildNode child);
        bool IsParent(std::string_view parentName, ChildNode child);

        bool IsAncestor(const Bus* ancestor, ChildNode child); // Checks if Bus is parent, grandparent, etc. of child
        bool IsAncestor(Index ancestorIndex, ChildNode child); // Checks if Bus is parent, grandparent, etc. of child
        bool IsAncestor(std::string_view ancestorName, ChildNode child); // Checks if Bus is parent, grandparent, etc. of child

        size_t BusCount();
    private:
        uint32_t m_Epoch = 0;
        bool m_IsDense = true;

        std::vector<std::optional<Bus>> m_Buses;
        std::vector<const char*> m_BusNames;
        std::unordered_map<std::string_view, Index> m_BusIndices;

        std::string_view StoreName(std::string_view name);
        std::string_view StoreName(std::string_view name, uint32_t index);
    };
}