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
		glm::vec3 cameraUp;
		glm::vec3 cameraRight;
		glm::vec3 worldUp;

		const float movementSpeed;
		const float mouseSensitivity;
		float cameraYaw;
		float cameraPitch;

		void updateCameraVectors();
	public:
		glm::vec3 cameraFront;
		glm::vec3 cameraPosition;
		
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
		float cameraYaw;
		const float reciprocalCentre;
		const float halfWidth;
		const float halfHeight;
	public: 
		glm::vec3 cameraPosition;
		glm::vec3 cameraDirection;
		glm::vec3 cameraPlane;
		
		RaycasterCamera(glm::vec3 playerPosition, float yaw, float centre, float width, float height);
		RaycasterCamera(float posX, float posY, float yaw, float centre, float width, float height);

		void UpdateCamera(const glm::vec3& playerPosition, float yaw);
		void UpdateCamera(float posX, float posY, float yaw);
	};
}

