#pragma once

#include "Types.h"
#include "Bus.h"

#include <unordered_map>

namespace Core::Audio {
	class BusManager {
	public:
        BusManager(uint32_t initialCapacity) { m_Buses.reserve(initialCapacity); m_BusIndices.reserve(initialCapacity); m_BusNames.reserve(initialCapacity); }

        ~BusManager();

        BusManager(const BusManager& other) = delete;
        BusManager(BusManager&& other) noexcept = delete;
        BusManager& operator = (const BusManager& other) = delete;
        BusManager& operator = (const BusManager&& other) = delete;

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
        
        Bus* GetBus(Index index);
        Bus* GetBus(std::string_view name);

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