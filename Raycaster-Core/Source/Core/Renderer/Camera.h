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
		virtual const glm::mat4& GetViewMatrix() const = 0;
	};

	class FlyCamera : public Camera
	{
	public:
		FlyCamera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
		FlyCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

		inline const glm::mat4& GetViewMatrix() const override { return m_View; };
		inline const glm::vec3& GetPosition() const { return m_Position; }
		inline const glm::vec3& GetDirection() const { return m_Direction; }
		inline float GetFov() const { return m_Fov; }

		void ProcessKeyboard(CameraMovement direction, float deltaTime);
		void ProcessMouseMovement(float offsetX, float offsetY, bool constrainPitch);
		void ProcessMouseScroll(float offset);

		void UpdateCamera(const glm::vec3& position, float yaw = 0, float pitch = 0);
	private:
		glm::mat4 m_View;

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
	};

	class Camera2D : public Camera {
	public: 
		Camera2D(const glm::vec3& position, float yaw, float zoom);

		inline const glm::mat4& GetViewMatrix() const override { return m_View; };
		inline const glm::vec3& GetPosition() const { return m_Position; } 
		inline const glm::vec3& GetDirection() const { return m_Direction; }
		inline const glm::vec3& GetPlane() const { return m_Plane; }

		void UpdateCamera(const glm::vec3& position, float yaw);
		void UpdateCamera(float zoom);
	private:
		glm::mat4 m_View;

		glm::vec3 m_Position;
		glm::vec3 m_Direction;
		glm::vec3 m_Plane;

		float m_Zoom;
	};
}

