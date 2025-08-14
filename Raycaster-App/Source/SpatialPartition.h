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
    void Init(const Map& map);
    void Shutdown();

    void Partition(std::span<const Value_t> values);
    void Move(Index_t index, glm::vec2 oldPosition, glm::vec2 newPosition);
    void Clear();

    struct Range {
        Index_t Start = static_cast<Index_t>(-1);
        Index_t Count = 0;
    };

    std::span<const Index_t> Get(glm::uvec2 position) const;
    std::span<const Index_t> GetRow(glm::uvec2 position, size_t rowLenght) const;
private:
    inline size_t CalculateIndex(size_t x, size_t y) const;

    std::vector<Index_t> m_Sorted;

    std::vector<Range> m_Grid;

    size_t m_GridWidth = 0;
};

template <HasPosition Value_t, std::integral Index_t>
void SpatialPartition<Value_t, Index_t>::Init(const Map& map) {
    m_GridWidth = map.GetWidth();
    m_Grid.assign(map.GetSize(), {});
}

template <HasPosition Value_t, std::integral Index_t>
void SpatialPartition<Value_t, Index_t>::Shutdown() {
    m_Grid.clear();
    m_Grid.shrink_to_fit();
    m_Sorted.clear();
    m_Sorted.shrink_to_fit();
}

template <HasPosition Value_t, std::integral Index_t>
void SpatialPartition<Value_t, Index_t>::Partition(std::span<const Value_t> values) {
    Clear();
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
        size_t spatialIndex = CalculateIndex(static_cast<size_t>(values[valueIndex].Position.x), static_cast<size_t>(values[valueIndex].Position.y));

        RC_ASSERT(spatialIndex < m_Grid.size());

        if (!m_Grid[spatialIndex].Count) {
            m_Grid[spatialIndex].Start = static_cast<Index_t>(i);
        }
        m_Grid[spatialIndex].Count++;
    }
}

template <HasPosition Value_t, std::integral Index_t>
void SpatialPartition<Value_t, Index_t>::Move(Index_t index, glm::vec2 oldPosition, glm::vec2 newPosition) {
    RC_ASSERT(index < m_Sorted.size());

    size_t oldIndex = CalculateIndex(oldPosition.x, oldPosition.y);
    size_t newIndex = CalculateIndex(newPosition.x, newPosition.y);
    bool shiftDirection = newIndex > oldIndex;

    RC_ASSERT(newIndex < m_Grid.size());
    if (oldIndex == newIndex) {
        return;
    }

    size_t movedIndex = -1;
    // Remove element from old position
    {
        auto& bucket = m_Grid[oldIndex];
        RC_ASSERT(bucket.Count, "Could not find element in SpatialPartion");

        auto iter = std::find(m_Sorted.begin() + bucket.Start, m_Sorted.begin() + bucket.Start + bucket.Count, index);
        RC_ASSERT(iter != m_Sorted.begin() + bucket.Start + bucket.Count, "Could not find element in SpatialPartion");

        movedIndex = shiftDirection? bucket.Start + bucket.Count - 1 : bucket.Start;
        *iter = m_Sorted[movedIndex]; // Swap with first/last element in bucket

        if (--bucket.Count) {
            bucket.Start += (movedIndex == bucket.Start);
        }
    }

    // Move elements between old and new position
    {
        int64_t sign = shiftDirection ? 1 : -1;

        for (int64_t i = oldIndex + sign; i * sign < static_cast<int64_t>(newIndex) * sign; i += sign) {
            auto& bucket = m_Grid[i];
            if (!bucket.Count) {
                continue;
            }

            movedIndex = bucket.Start + shiftDirection* bucket.Count - shiftDirection;
            size_t to = bucket.Start + !shiftDirection* bucket.Count - shiftDirection;
            m_Sorted[to] = m_Sorted[movedIndex];
            bucket.Start -= sign;
        }
    }

    // Add element to new position
    {
        auto& bucket = m_Grid[newIndex];
        if (bucket.Count) {
            bucket.Start -= shiftDirection;
            movedIndex = bucket.Start + !shiftDirection * bucket.Count;
        } else {
            bucket.Start = movedIndex;
        }

        m_Sorted[movedIndex] = index;
        bucket.Count++;
    }
}

template <HasPosition Value_t, std::integral Index_t>
void SpatialPartition<Value_t, Index_t>::Clear() {
    m_Sorted.clear();
    m_Grid.assign(m_Grid.size(), {});
}

template <HasPosition Value_t, std::integral Index_t>
std::span<const Index_t> SpatialPartition<Value_t, Index_t>::Get(glm::uvec2 position) const {
    std::span<const Index_t> result{};
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
std::span<const Index_t> SpatialPartition<Value_t, Index_t>::GetRow(glm::uvec2 position, size_t rowLength) const {
    std::span<const Index_t> result{};
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
inline size_t SpatialPartition<Value_t, Index_t>::CalculateIndex(size_t x, size_t y) const {
    return y * m_GridWidth + x;
}
