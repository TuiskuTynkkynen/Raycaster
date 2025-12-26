#pragma once

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

    void Render(const Map& map, const Core::Camera2D& camera, const Player& player, Renderables& renderables);

    void RenderWalls(const Map& map, const Core::Camera2D& camera);
    void RenderFloors(const Map& map, const Core::Camera2D& camera);
    void RenderSprites(const Map& map, const Player& player, Renderables& renderables);
    void RenderInventory(const Map& map, const Player& player, Renderables& renderables);

    inline std::span<const Ray> GetRays() const { return m_Rays; }
    inline std::span<const Line> GetLines() const { return m_Lines; }
    inline std::span<const Floor> GetFloors() const { return m_Floors; }

    inline static constexpr size_t GetRayCount() { return m_RayCount; }
    inline static constexpr float GetRayWidth() { return m_RayWidth; }

    inline void SetAspecRatio(float aspectRatio) { m_AspectRatio = aspectRatio; m_ReciprocalAspectRatio = 1.0f / aspectRatio; };
private:
    static constexpr size_t m_RayCount = 500;
    static constexpr float m_RayWidth = 2.0f / m_RayCount; // Screen is 2.0f wide/tall
    static constexpr bool m_SnappingEnabled = true;
    
    std::vector<Ray> m_Rays;
    std::vector<Line> m_Lines;
    std::vector<Floor> m_Floors;
    std::array<float, m_RayCount> m_ZBuffer{};

    float m_AspectRatio = 1.0f;
    float m_ReciprocalAspectRatio = 1.0f;
};
