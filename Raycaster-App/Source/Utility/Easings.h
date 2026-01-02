#pragma once

#include <glm/glm.hpp>

namespace Easings {
    float EaseOutBounce(float t) {
        float bounce1 = -4.0f * (t + 0.5f) * (t - 0.5f);
        float bounce2 = -16.0f * (t - 0.5f) * (t - 0.8f);
        float bounce3 = -8.0f * (t - 0.8f) * (t - 1.0f);

        return 1.f - glm::max(bounce1, glm::max(bounce2, bounce3));
    }
}