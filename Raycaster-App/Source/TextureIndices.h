#pragma once

#include "Core/Base/Timestep.h"

#include <cstdint>
#include <limits>

inline constexpr uint32_t ATLASWIDTH = 11;
inline constexpr uint32_t ATLASHEIGHT = 2;

namespace TextureIndices {
    enum Enumaration : uint32_t {
        Wall_0                         = 0,
        Wall_1                         = 1,
        Wall_2                         = 2,
        Wall_3                         = 3,
        Wall_4                         = 4,
        Wall_5                         = 5,
        Wall_6                         = 6,
        Wall_7                         = 7,
        Barrel                         = 8,
        Pilar                          = 9,
        Light                          = 10,
        Enemy_Basic                    = 11,
        Floor_Item_Dagger              = 14,
        Chest_Animation_Start          = 15,
        Attack_Animation_Dagger_Start  = 17,
    };
}

namespace TextureOffsets {
    enum Enumeration : uint32_t {
        Enemy_Walk   = 0,
        Enemy_Attack = 1,
        Enemy_Corpse = 2,
    };
}

struct AtlasAnimation {
    uint16_t StartAtlasIndex = 0;
    uint16_t FrameCount = 1;

    constexpr uint32_t GetFrame(Core::Timestep progress) const {
        // Subtract epsilon, since animation should be an exclusive range [StartAtlasIndex, StartAtlasIndex + Framecount)
        return StartAtlasIndex + static_cast<uint32_t>((FrameCount - std::numeric_limits<float>::epsilon()) * progress);
    }
};

namespace Animations {
    inline constexpr AtlasAnimation ChestOpen = AtlasAnimation{ TextureIndices::Chest_Animation_Start, 2 };
    inline constexpr AtlasAnimation AttackDagger = AtlasAnimation{ TextureIndices::Attack_Animation_Dagger_Start, 5 };
}