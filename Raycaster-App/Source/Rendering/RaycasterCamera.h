#pragma once

#include "Core/Renderer/Camera.h"

class RaycasterCamera : public Core::Camera2D {
public:
	RaycasterCamera(glm::vec3 position, float zoom, float yaw, float pitch);

	inline const glm::vec3& GetDirection() const { return m_Direction; }
	inline const glm::vec3& GetPlane() const { return m_Plane; }
	inline float GetYaw() const { return m_Yaw; }
	inline float GetPitch() const { return m_Pitch; }

	void UpdateCamera(glm::vec3 position, float yaw, float pitch);
private:
	glm::vec3 m_Direction;
	glm::vec3 m_Plane;
	float m_Yaw;
	float m_Pitch;
};