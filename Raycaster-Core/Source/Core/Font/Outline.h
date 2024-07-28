#pragma once

#include "Core/Debug/Debug.h"

#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BBOX_H
#include FT_OUTLINE_H

#include <vector>
namespace Core::SDF {
    class Outline {
    public:
        void Init(const FT_BBox& bounds, FT_Outline* ftOutline);

        float DistanceTo(const glm::vec2& point);
        bool ContainsPoint(const glm::vec2& p);
    private:
        class Bezier {
        public:
            uint32_t PointsStart;
            uint32_t PointsEnd;

            uint32_t LUTStart = 0;
            float Lenght = 0.0f;

            Bezier(uint32_t start, uint32_t end)
                : PointsStart(start), PointsEnd(end) {}

            inline glm::vec2 Compute(float t, const std::vector<glm::vec2>& points) const {
                const uint32_t order = PointsEnd - PointsStart;
                RC_ASSERT(order == 2 || order == 3, "Only quadratic and cubic beziers are supported");
                return order == 2 ? ComputeQuadratic(t, points) : ComputeCubic(t, points);
            }
        private:
            glm::vec2 ComputeQuadratic(float t, const std::vector<glm::vec2>& points) const;
            glm::vec2 ComputeCubic(float t, const std::vector<glm::vec2>& points) const;
        };
        struct Contour {
            uint32_t BeziersStart;
            uint32_t BeziersEnd;

            uint32_t LinesStart;
            uint32_t LinesEnd;


            Contour(uint32_t beziersStart, uint32_t linesStart)
                : BeziersStart(beziersStart), BeziersEnd(beziersStart), LinesStart(linesStart), LinesEnd(linesStart) {}
        };

        uint32_t IterationCount = 0;
        float StepSize = 0.0f;

        std::vector<Contour> Contours;

        std::vector<glm::uvec2> Lines;
        std::vector<Bezier> Beziers;

        std::vector<glm::vec2> Points;
        std::vector<glm::vec2> LUT;

        FT_BBox Bounds = {};

        glm::vec2 CalculatePixelPositions(const FT_Vector& point) const;
        void ComputeLUT(Bezier& bezier);
        

        float DistanceToLine(const glm::vec2& point, const glm::vec2& lineStart, const glm::vec2& lineEnd);

        uint32_t LineIntersection(const glm::vec2& lineStart, const glm::vec2& lineEnd, const glm::vec2& point);
    };
}
