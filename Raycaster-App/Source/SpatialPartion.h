#pragma once

#include "Map.h"

#include <span>
#include <ranges>
#include <numeric>

template <typename T>
concept HasPosition = std::same_as<decltype(std::declval<T>().Position), glm::vec2>;

template <HasPosition Value_t, std::integral Index_t = size_t>
class SpatialPartition {
public:
    SpatialPartition(const Map& map);

    void Partition(std::span<const Value_t> values);
    void Move(Index_t index, glm::vec2 oldPosition, glm::vec2 newPosition);
    void Clear();

    struct Range {
        Index_t Start = static_cast<Index_t>(-1);
        Index_t Count = 0;
    };

    std::span<Index_t> Get(glm::uvec2 position);
    std::span<Index_t> GetRow(glm::uvec2 position, size_t rowLenght);
private:
    inline size_t CalculateIndex(size_t x, size_t y);

    std::vector<Index_t> m_Sorted;

    std::vector<Range> m_Grid;

    size_t m_GridWidth = 0;
};

template <HasPosition Value_t, std::integral Index_t>
SpatialPartition<Value_t, Index_t>::SpatialPartition(const Map& map)
    : m_GridWidth(map.GetWidth()) {
    m_Grid.assign(map.GetSize(), {});
}

template <HasPosition Value_t, std::integral Index_t>
void SpatialPartition<Value_t, Index_t>::Partition(std::span<const Value_t> values) {
    RC_ASSERT(values.size() < std::numeric_limits<Index_t>::max());

    m_Sorted.resize(values.size());
    std::iota(m_Sorted.begin(), m_Sorted.end(), 0);

    std::ranges::sort(m_Sorted, [values](size_t A, size_t B) {
        glm::uvec2 APosition = values[A].Position;
        glm::uvec2 BPosition = values[B].Position;

        return APosition.y < BPosition.y  || (APosition.y == BPosition.y && APosition.x < BPosition.x);
        });
    
    for (size_t i = 0; i < m_Sorted.size(); i++) {
        size_t valueIndex = m_Sorted[i];
        size_t spatialIndex = CalculateIndex(values[valueIndex].Position.x, values[valueIndex].Position.y);

        RC_ASSERT(spatialIndex < m_Grid.size());

        if (!m_Grid[spatialIndex].Count) {
            m_Grid[spatialIndex].Start = i;
        }
        m_Grid[spatialIndex].Count++;
    }
}

template <HasPosition Value_t, std::integral Index_t>
void SpatialPartition<Value_t, Index_t>::Clear() {
    m_Sorted.clear();
    m_Grid.assign(m_Grid.size(), {});
}

template <HasPosition Value_t, std::integral Index_t>
std::span<Index_t> SpatialPartition<Value_t, Index_t>::Get(glm::uvec2 position) {
    std::span<Index_t> result{};
    size_t index = CalculateIndex(position.x, position.y);
    
    if (index >= m_Grid.size()) {
        return result;
    }
    
    if (Range range = m_Grid[index]; range.Count) {
        RC_ASSERT(range.Start + range.Count <= m_Sorted.size());
        result = { m_Sorted.begin() + range.Start, range.Count };
    }

    return result;
}

template <HasPosition Value_t, std::integral Index_t>
std::span<Index_t> SpatialPartition<Value_t, Index_t>::GetRow(glm::uvec2 position, size_t rowLength) {
    std::span<Index_t> result{};
    if (position.y * m_GridWidth >= m_Grid.size()) {
        return result;
    }

    Range range{};

    size_t maxExtent = rowLength / 2;
    size_t minExtent = rowLength - maxExtent - 1;
    for (size_t i = glm::clamp(position.x - minExtent, static_cast<size_t>(0), m_GridWidth); i <= glm::min(position.x + maxExtent, m_GridWidth - 1); i++) {
        size_t spatialIndex = CalculateIndex(i, position.y);

        if (m_Grid[spatialIndex].Count && m_Grid[spatialIndex].Start < range.Start) {
            range.Start = m_Grid[spatialIndex].Start;
        }

        range.Count += m_Grid[spatialIndex].Count;
    }

    if (range.Count) {
        RC_ASSERT(range.Start + range.Count <= m_Sorted.size());
        result = { m_Sorted.begin() + range.Start, range.Count };
    }

    return result;
}

template <HasPosition Value_t, std::integral Index_t>
inline size_t SpatialPartition<Value_t, Index_t>::CalculateIndex(size_t x, size_t y) {
    return y * m_GridWidth + x;
}
