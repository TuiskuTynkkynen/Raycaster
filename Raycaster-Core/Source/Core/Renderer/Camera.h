#pragma once

#include <glm/glm.hpp>

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 5.0f;
const float SENSITIVITY = 0.1f;
const float FOV = 45.0f;

namespace Core {
	enum CameraMovement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		UP,
		DOWN,
	};

	class Camera {
	public:
		virtual glm::mat4 GetViewMatrix() = 0;
	};

	class FlyCamera : public Camera
	{
	private:
		glm::vec3 m_CameraUp;
		glm::vec3 m_CameraRight;
		glm::vec3 m_WorldUp;

		glm::vec3 m_Direction;
		glm::vec3 m_Position;
		
		float m_Fov;
		const float m_MovementSpeed;
		const float m_MouseSensitivity;
		float m_CameraYaw;
		float m_CameraPitch;

		void UpdateCameraVectors();
	public:

		FlyCamera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
		FlyCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

		glm::mat4 GetViewMatrix();
		const glm::vec3& GetPosition() { return m_Position; }
		const glm::vec3& GetDirection() { return m_Direction; }
		float GetFov() { return m_Fov; }

		void ProcessKeyboard(CameraMovement direction, float deltaTime);
		void ProcessMouseMovement(float offsetX, float offsetY, bool constrainPitch);
		void ProcessMouseScroll(float offset);

		void UpdateCamera(const glm::vec3& position, float yaw = 0, float pitch = 0);
	};

	class RaycasterCamera : public Camera { //2D Camera
	private:
		glm::vec3 m_Position;
		glm::vec3 m_Direction;
		glm::vec3 m_Plane;

		float m_CameraYaw;
		const float m_ReciprocalCentre;
		const float m_HalfWidth;
		const float m_HalfHeight;
	public: 
		RaycasterCamera(const glm::vec3& playerPosition, float yaw, float centre, float width, float height);
		RaycasterCamera(float posX, float posY, float yaw, float centre, float width, float height);

		glm::mat4 GetViewMatrix() override;
		const glm::vec3& GetPosition() { return m_Position; }
		const glm::vec3& GetDirection() { return m_Direction; }
		const glm::vec3& GetPlane() { return m_Plane; }

		void UpdateCamera(const glm::vec3& playerPosition, float yaw);
		void UpdateCamera(float posX, float posY, float yaw);
	};
}

