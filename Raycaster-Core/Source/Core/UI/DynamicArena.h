#pragma once

#include "Core/Debug/Assert.h"

#include <array>
#include <cstddef>
#include <limits>
#include <memory>
#include <vector>

namespace Core {
    template<size_t BlockSize = 4096>
    class DynamicArena {
    public:
        template <typename T>
        [[nodiscard]] inline T* Allocate(size_t n = 1) requires (!std::is_void_v<T> && sizeof(T) <= BlockSize) {
            if (n > std::numeric_limits<size_t>::max() / sizeof(T)) throw std::bad_array_new_length();
            return reinterpret_cast<T*>(AllocateBytes(n * sizeof(T), alignof(T)));
        }
        template <typename T>
        inline void Deallocate(T*) noexcept {}; 
        
        // Prepares DynamicArena for reuse, does not call object destructors.
        inline void DeallocateAll() { 
            if (!m_Blocks.empty()) { m_Blocks[0]->Free = BlockSize; }
            m_CurrentBlock = 0;
        }

        // Releases unused memory back to global allocator.
        inline void ReleaseUnused() {
            if (m_CurrentBlock + 1 < m_Blocks.size()) { m_Blocks.resize(m_CurrentBlock + 1); }
            m_Blocks.shrink_to_fit();
        }
        // Releases all memory back to global allocator, does not call object destructors.
        inline void ReleaseAll() { DeallocateAll(); m_Blocks = std::vector<std::unique_ptr<Block>>{}; }
    private:
        void* AllocateBytes(size_t count, size_t alignment) {
            RC_ASSERT(alignment != 0 && (alignment & (alignment - 1)) == 0); // Alignment must be power of two
            if (BlockSize < count) { throw std::bad_alloc(); } // Never fits
            if (m_CurrentBlock >= m_Blocks.size() || m_Blocks[m_CurrentBlock]->Free < AlignedSize(m_Blocks[m_CurrentBlock]->Data(), count, alignment)) {
                AddBlock();
            }

            void* result = m_Blocks[m_CurrentBlock]->Data();
            if (std::align(alignment, count, result, m_Blocks[m_CurrentBlock]->Free) != nullptr) {
                m_Blocks[m_CurrentBlock]->Free -= count;
                return result;
            }

            // Prevents empty blocks with repeated allocations where aligned_size > BlockSize.
            if (m_CurrentBlock && m_Blocks[m_CurrentBlock]->Free == BlockSize) { m_CurrentBlock--; }
            throw std::bad_alloc(); // Can not fit due to alignment
        }

        inline size_t AlignedSize(const void* ptr, size_t size, size_t alignment) {
            size_t padding = reinterpret_cast<uintptr_t>(ptr) & (alignment - 1);
            if (padding != 0) { padding = alignment - padding; }
            return size + padding;
        }

        inline void AddBlock() {
            if (++m_CurrentBlock < m_Blocks.size()) {
                m_Blocks[m_CurrentBlock]->Free = BlockSize;
                return;
            }
            m_CurrentBlock = m_Blocks.size();
            m_Blocks.emplace_back(std::make_unique<Block>());    
        }

        struct Block {
            size_t Free = BlockSize;
            alignas(alignof(std::max_align_t))
            std::array<std::byte, BlockSize> Memory{};
            inline void* Data() { return Memory.data() + BlockSize - Free; }
        };

        size_t m_CurrentBlock = 0;
        std::vector<std::unique_ptr<Block>> m_Blocks{};
    };
}
