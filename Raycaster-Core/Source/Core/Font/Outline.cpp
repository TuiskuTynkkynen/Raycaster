#include "Outline.h"

namespace Core::SDF {
    void Outline::Init(const FT_BBox& bounds, FT_Outline* ftOutline) {
        Bounds = bounds;

        int32_t x = (bounds.xMax - bounds.xMin) >> 6;
        int32_t y = (bounds.yMax - bounds.yMin) >> 6;

        IterationCount = sqrt(x * x + y * y) * 2;
        StepSize = 1.0f / IterationCount;


        Points.resize(ftOutline->n_points);
        Contours.clear();
        Lines.clear();
        Beziers.clear();
        LUT.clear();

        static constexpr FT_Outline_Funcs func_interface = {
            .move_to = [](const FT_Vector* to, void* ptr) -> int {
                Outline* outline = (Outline*)ptr;

                outline->Points.push_back(outline->CalculatePixelPositions(*to));
                outline->Contours.emplace_back(outline->Beziers.size(), outline->Lines.size());

                return 0;
            },
            .line_to = [](const FT_Vector* to, void* ptr) -> int {
                Outline* outline = (Outline*)ptr;

                outline->Points.push_back(outline->CalculatePixelPositions(*to));
                outline->Lines.emplace_back(outline->Points.size() - 2, outline->Points.size() - 1);

                outline->Contours.back().LinesEnd++;
                return 0;
            },
            .conic_to = [](const FT_Vector* control, const FT_Vector* to, void* ptr) -> int {
                Outline* outline = (Outline*)ptr;

                outline->Points.push_back(outline->CalculatePixelPositions(*control));
                outline->Points.push_back(outline->CalculatePixelPositions(*to));
                outline->Beziers.emplace_back(outline->Points.size() - 3, outline->Points.size() - 1);

                outline->Contours.back().BeziersEnd++;
                return 0;
            },
            .cubic_to = [](const FT_Vector* c1, const FT_Vector* c2, const FT_Vector* to, void* ptr) -> int {
                Outline* outline = (Outline*)ptr;

                outline->Points.push_back(outline->CalculatePixelPositions(*c1));
                outline->Points.push_back(outline->CalculatePixelPositions(*c2));
                outline->Points.push_back(outline->CalculatePixelPositions(*to));
                outline->Beziers.emplace_back(outline->Points.size() - 4, outline->Points.size() - 1);

                outline->Contours.back().BeziersEnd++;
                return 0;
            },
            .shift = 0,
            .delta = 0
        };
        FT_Outline_Decompose(ftOutline, &func_interface, this);
    }

    float Outline::DistanceTo(const glm::vec2& point) {
        //Get minimum distance to all lines
        float minLineDistance = FLT_MAX;
        for (glm::uvec2 line : Lines) {
            const float distance = DistanceToLine(point, Points[line.x], Points[line.y]);
            if (distance < minLineDistance) {
                minLineDistance = distance;
            }
        }

        if (minLineDistance <= 1e-3f || Beziers.size() == 0) {
            return minLineDistance;
        }

        //Approximate minimum distance to all beziers
        float minBezierDistance = FLT_MAX;

        uint32_t minBezierIndex = 0;
        uint32_t minLutIndex = 0;
        for (uint32_t i = 0; i < Beziers.size(); i++) {
            if (Beziers[i].Lenght == 0.0f) {
                ComputeLUT(Beziers[i]);
            }
            RC_ASSERT(LUT.size() >= Beziers[i].LUTStart + IterationCount);

            for (uint32_t j = Beziers[i].LUTStart; j < Beziers[i].LUTStart + IterationCount; j++) {
                const glm::vec2 temp = point - LUT[j];
                const float distance = sqrt(temp.x * temp.x + temp.y * temp.y);

                if (distance < minBezierDistance) {
                    minBezierDistance = distance;
                    minBezierIndex = i;
                    minLutIndex = j;
                }
            }
        }

        const float approximateError = Beziers[minBezierIndex].Lenght * StepSize;
        if (minLineDistance <= minBezierDistance - approximateError) {
            return minLineDistance;
        }

        //Better approximate distance to the closest bezier
        const float t1 = float(minLutIndex + 1) / IterationCount;
        const float step = StepSize / IterationCount;

        const Bezier bezier = Beziers[minBezierIndex];
        for (float t = float(minLutIndex - 1) / IterationCount; t < t1; t += step) {
            const glm::vec2 temp = point - bezier.Compute(t, Points);
            const float distance = sqrt(temp.x * temp.x + temp.y * temp.y);

            if (distance < minBezierDistance) {
                minBezierDistance = distance;
            }
        }

        return minBezierDistance;
    }

    bool Outline::ContainsPoint(const glm::vec2& point) {
        bool result = false;

        //Each contour checked indevidually, since only contours can't self intersect 
        for (uint32_t contour = 0; contour < Contours.size(); contour++) {
            uint32_t intersectionCount = 0;

            for (uint32_t i = Contours[contour].LinesStart; i < Contours[contour].LinesEnd; i++) {
                const glm::uvec2& line = Lines[i];
                intersectionCount += LineIntersection(Points[line.x], Points[line.y], point);
            }
            for (uint32_t i = Contours[contour].BeziersStart; i < Contours[contour].BeziersEnd; i++) {
                const Bezier& bezier = Beziers[i];

                if (bezier.Lenght == 0.0f) {
                    ComputeLUT(Beziers[i]);
                }
                RC_ASSERT(LUT.size() >= Beziers[i].LUTStart + IterationCount);

                for (uint32_t j = 0; j < IterationCount - 1; j++) {
                    intersectionCount += LineIntersection(LUT[bezier.LUTStart + j], LUT[bezier.LUTStart + j + 1], point);
                }
            }

            result ^= (intersectionCount % 2 == 1);
        }

        return  result;
    }

    uint32_t Outline::LineIntersection(const glm::vec2& lineStart, const glm::vec2& lineEnd, const glm::vec2& point) {
        bool isDownward = lineStart.y > lineEnd.y;

        if (isDownward) {
            if (lineStart.y < point.y && lineEnd.y <= point.y) return 0;
            if (lineStart.y > point.y && lineEnd.y >= point.y) return 0;
        }
        else {
            if (lineStart.y <= point.y && lineEnd.y < point.y) return 0;
            if (lineStart.y >= point.y && lineEnd.y > point.y) return 0;
        }

        const glm::vec2 s1 = lineEnd - lineStart;
        const glm::vec2 s2(Bounds.xMax, 0.0f);

        const float s = (-s1.y * (lineStart.x - point.x) + s1.x * (lineStart.y - point.y)) / (-s2.x * s1.y + s1.x * s2.y);

        if (s >= 0 && s <= 1) {
            return 1;
        }

        return 0;
    }
    
    float Outline::DistanceToLine(const glm::vec2& point, const glm::vec2& lineStart, const glm::vec2& lineEnd) {
        const glm::vec2& p1 = lineStart;
        const glm::vec2& p2 = lineEnd;

        auto dp = p2 - p1;
        float u = ((point.x - p1.x) * dp.x + (point.y - p1.y) * dp.y) / (dp.x * dp.x + dp.y * dp.y);
        u = std::clamp(u, 0.0f, 1.0f);

        float dx = p1.x + u * dp.x - point.x;
        float dy = p1.y + u * dp.y - point.y;

        float dist = sqrt(dx * dx + dy * dy);

        return dist;
    }

    glm::vec2 Outline::CalculatePixelPositions(const FT_Vector& point) const {
        RC_ASSERT("", Bounds.xMax - Bounds.xMin != 0 || Bounds.yMax - Bounds.yMin != 0);

        float x = (point.x - Bounds.xMin) / 64.0f;
        float y = (Bounds.yMax - point.y) / 64.0f;

        return glm::vec2(x, y);
    }

    void Outline::ComputeLUT(Bezier& bezier) {
        bezier.LUTStart = LUT.size();

        glm::vec2 current{};
        for (uint32_t i = 0; i < IterationCount - 1; i++) {
            current = bezier.Compute(i * StepSize, Points);

            if (i != 0) {
                bezier.Lenght += glm::distance(current, LUT.back());
            }

            LUT.push_back(current);
        }

        LUT.push_back(Points[bezier.PointsEnd]);
    }

    glm::vec2 Outline::Bezier::ComputeQuadratic(float t, const std::vector<glm::vec2>& points) const {
        RC_ASSERT(points.size() > PointsStart + 2);
        if (t <= 0.0f) {
            return points[PointsStart];
        }
        if (t >= 1.0f) {
            return points[PointsStart + 2];
        }

        const float mt = 1 - t;

        const float a = mt * mt;
        const float b = mt * t * 2;
        const float c = t * t;

        return points[PointsStart] * a + points[PointsStart + 1] * b + points[PointsStart + 2] * c;
    }

    glm::vec2 Outline::Bezier::ComputeCubic(float t, const std::vector<glm::vec2>& points) const {
        RC_ASSERT(points.size() > PointsStart + 3);
        if (t <= 0.0f) {
            return points[PointsStart];
        }
        if (t >= 1.0f) {
            return points[PointsStart + 2];
        }

        const float mt = 1 - t;
        const float t2 = t * t;
        const float mt2 = mt * mt;

        const float a = mt2 * mt;
        const float b = mt2 * t * 3;
        const float c = mt * t2 * 3;
        const float d = t * t2;

        return points[PointsStart] * a + points[PointsStart + 1] * b + points[PointsStart + 2] * c + points[PointsStart + 3] * d;
    }
}