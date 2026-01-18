#pragma once

#include "RaycasterCamera.h"
#include "Entities.h"
#include "Map.h"
#include "Renderables.h"
#include "Player.h"

#include <array>
#include <vector>

class RaycastRenderer {
public:
    RaycastRenderer() {
        m_Rays.resize(m_RayCount);
        m_Lines.resize(m_RayCount);
    }

    void Render(const Map& map, const RaycasterCamera& camera, const Player& player, Renderables& renderables);

    void RenderWalls(const Map& map, const RaycasterCamera& camera);
    void RenderFloors(const Map& map, const RaycasterCamera& camera);
    void RenderSprites(const Map& map, const Player& player, Renderables& renderables);
    
    inline std::span<const Ray> GetRays() const { return m_Rays; }
    inline std::span<const Line> GetLines() const { return m_Lines; }
    inline std::span<const Floor> GetFloors() const { return m_Floors; }

    inline static constexpr size_t GetRayCount() { return m_RayCount; }
    inline static constexpr float GetRayWidth() { return m_RayWidth; }

    inline void SetAspecRatio(float aspectRatio) { m_AspectRatio = aspectRatio; m_ReciprocalAspectRatio = 1.0f / aspectRatio; };
private:
    void RenderFloor(bool ceiling, const Map& map, const RaycasterCamera& camera);
    static constexpr size_t m_RayCount = 500;
    static constexpr float m_RayWidth = 2.0f / m_RayCount; // Screen is 2.0f wide/tall
    static constexpr bool m_SnappingEnabled = true;
    
    std::vector<Ray> m_Rays;
    std::vector<Line> m_Lines;
    std::vector<Floor> m_Floors;
    std::array<float, m_RayCount> m_ZBuffer{};
    std::array<std::vector<uint32_t>, m_RayCount / 2> m_DepthMap;

    float m_AspectRatio = 1.0f;
    float m_ReciprocalAspectRatio = 1.0f;
};
