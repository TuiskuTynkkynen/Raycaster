#pragma once
#include "Core.h"

#include "Entities.h"
#include "Algorithms.h"

#include <glm/glm.hpp>

#include <vector>
#include <array>
#include <span>

class  Map {
public:
    std::vector<LineCollider> CreateWalls();
    std::vector<Tile> CreateTiles();
    Core::Model CreateModel(const std::span<LineCollider> walls, std::shared_ptr<Core::Texture2D> atlas, std::shared_ptr<Core::Shader> shader);
 
    struct HitInfo {
        float Distance = 0.0f;
        uint8_t Side = 0; // 0 -> x, 1 -> y, 2 -> diagonal
        uint8_t Material = 0;
        glm::vec2 WorlPosition{};
    };
    HitInfo CastRay(glm::vec3 origin, glm::vec3 direction);

    struct FloorHitInfo {
        float Distance = 0.0f;
        uint8_t Side = 0; // 0 -> x, 1 -> y, >= 2 -> off the map
        uint8_t BottomMaterial = 0;
        uint8_t TopMaterial = 0;
        glm::vec2 WorlPosition{};
    };
    FloorHitInfo CastFloors(glm::vec2 origin, glm::vec3 reciprocalDirection, float maxDistance);

    static constexpr uint32_t GetHeight() { return s_MapData.Height; }
    static constexpr uint32_t GetWidth() { return s_MapData.Width; }
    static constexpr uint32_t GetSize() { return s_MapData.Size; }

    static constexpr float GetScalingFactor() { return s_MapData.ScalingFactor; }
    static constexpr glm::vec3 GetScale() { return s_MapData.Scale; }

    uint8_t operator [](size_t index){
        if (index < s_MapData.Size) {
            return s_MapData.Map[index];
        }

        return 0;
    }
private:
    struct MapData {
        static constexpr uint32_t Height = 24, Width = 24;
        static constexpr uint32_t Size = Height * Width;

        static constexpr std::array<int8_t, Size> Map {
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,1,
            1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,-2,2,2,2,-2,0,0,0,0,3,0,3,0,3,0,0,0,1,
            1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1,
            1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,-2,2,0,2,-2,0,0,0,0,3,0,3,0,3,0,0,0,1,
            1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-5,-5,0,0,0,0,0,1,
            1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,-5,-5,0,0,0,0,0,1,
            1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
            1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
        };

        static constexpr std::array<uint8_t, Size> FloorMap {
            7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
            7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
            7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
            7,7,7,7,7,7,7,7,7,7,7,6,7,7,7,7,7,7,7,7,7,7,7,7,
            7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
            7,7,7,7,7,7,7,5,3,5,7,7,7,7,7,7,7,4,7,7,7,7,7,7,
            7,7,7,7,7,7,7,3,5,3,7,7,7,7,7,7,4,4,4,7,7,7,7,7,
            7,7,7,7,7,7,7,5,3,5,7,7,7,7,7,7,7,4,7,7,7,7,7,7,
            7,7,7,7,7,7,7,7,5,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
            7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
            7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
            7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
            7,7,7,7,7,7,7,7,7,7,7,5,7,7,7,7,7,7,7,7,7,7,7,7,
            7,7,7,7,7,7,7,7,7,7,7,5,7,7,7,7,7,7,7,7,7,7,7,7,
            7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
            7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,5,5,7,7,7,7,7,7,
            7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,5,5,7,7,7,7,7,7,
            7,7,1,7,1,1,1,1,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
            7,7,1,1,1,1,1,1,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
            7,7,1,7,1,1,1,1,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
            7,7,1,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
            7,7,1,1,1,1,1,1,1,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
            7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
            7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
        };

        static constexpr std::array<uint8_t, Size> CeilingMap {
            6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
            6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
            6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
            6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
            6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
            6,6,6,6,6,6,6,3,5,3,6,6,6,6,6,6,6,4,6,6,6,6,6,6,
            6,6,6,6,6,6,6,5,3,5,6,6,6,6,6,6,4,4,4,6,6,6,6,6,
            6,6,6,6,6,6,6,3,5,3,6,6,6,6,6,6,6,4,6,6,6,6,6,6,
            6,6,6,6,6,6,6,6,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
            6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
            6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
            6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
            6,6,6,6,6,6,6,6,6,6,5,6,6,6,6,6,6,6,6,6,6,6,6,6,
            6,6,6,6,6,6,6,6,6,6,5,6,6,6,6,6,6,6,6,6,6,6,6,6,
            6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
            6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,5,5,6,6,6,6,6,6,
            6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,5,5,6,6,6,6,6,6,
            6,6,1,6,1,1,1,1,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
            6,6,1,1,1,1,1,1,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
            6,6,1,6,1,1,1,1,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
            6,6,1,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
            6,6,1,1,1,1,1,1,1,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
            6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
            6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6
        };

        const float ScalingFactor = glm::sqrt((float)Size) / 1.4f;
        const glm::vec3 Scale = glm::vec3(1.0f / ScalingFactor, 1.0f / ScalingFactor, 0.0f);
    };
    inline static MapData s_MapData;

    union Neighbours {
        struct {
            bool Left   : 1;
            bool Down   : 1;
            bool Up     : 1;
            bool Right  : 1;
        };
        uint8_t Data;
        
        bool operator[](size_t index) {
            RC_ASSERT(index < 4, "Neighbour contains only 4 elements");
            return (Data >> index) & 1;
        }
    };
    constexpr Neighbours GetNeighbours(size_t index);
};