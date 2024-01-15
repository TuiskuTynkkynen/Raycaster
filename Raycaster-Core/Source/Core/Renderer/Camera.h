#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Core {
	enum CameraMovement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		UP,
		DOWN,
	};

	class FlyCamera
	{
	private:
		glm::vec3 m_cameraUp;
		glm::vec3 m_cameraRight;
		glm::vec3 m_worldUp;

		const float m_movementSpeed;
		const float m_mouseSensitivity;
		float m_cameraYaw;
		float m_cameraPitch;

		void updateCameraVectors();
	public:
		glm::vec3 direction;
		glm::vec3 position;
		
		float fov;

		FlyCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);
		FlyCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

		glm::mat4 GetViewMatrix();
		void ProcessKeyboard(CameraMovement direction, float deltaTime);
		void ProcessMouseMovement(float offsetX, float offsetY, bool constrainPitch);
		void ProcessMouseScroll(float offset);
	};

	class RaycasterCamera //2D Camera
	{
	private:
		float m_cameraYaw;
		const float m_reciprocalCentre;
		const float m_halfWidth;
		const float m_halfHeight;
	public: 
		glm::vec3 position;
		glm::vec3 direction;
		glm::vec3 plane;
		
		RaycasterCamera(glm::vec3 playerPosition, float yaw, float centre, float width, float height);
		RaycasterCamera(float posX, float posY, float yaw, float centre, float width, float height);

		glm::mat4 GetViewMatrix();

		void UpdateCamera(const glm::vec3& playerPosition, float yaw);
		void UpdateCamera(float posX, float posY, float yaw);
	};
}

