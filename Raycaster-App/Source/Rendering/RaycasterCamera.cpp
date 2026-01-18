#include "RaycasterCamera.h"

RaycasterCamera::RaycasterCamera(glm::vec3 position, float zoom, float yaw, float pitch)
    : Core::Camera2D(position, zoom) {
    UpdateCamera(position, yaw, pitch);
}

void RaycasterCamera::UpdateCamera(glm::vec3 position, float yaw, float pitch) {
    Core::Camera2D::UpdateCamera(position);

    glm::vec3 front(0.0f);
    yaw = glm::radians(yaw);
    front.x = glm::cos(yaw);
    front.y = glm::sin(yaw);

    m_Direction = glm::normalize(front);
    constexpr glm::vec3 worldUp = glm::vec3(0.0f, 0.0f, 1.0f);
    m_Plane = glm::normalize(glm::cross(m_Direction, worldUp));

    m_Direction.z = glm::tan(glm::radians(pitch));
}