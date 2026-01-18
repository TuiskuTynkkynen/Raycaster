#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Core {
    FlyCamera::FlyCamera(const glm::vec3& position, const glm::vec3& up, float yaw, float pitch)
        : m_Position(position), m_MovementSpeed(SPEED), m_MouseSensitivity(SENSITIVITY), m_Fov(FOV), m_WorldUp(up), m_CameraYaw(yaw), m_CameraPitch(pitch)
    {
       m_Direction = glm::vec3(0.0f, 0.0f, -1.0f);
       UpdateCameraVectors();
    }

    FlyCamera::FlyCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
        : m_MovementSpeed(SPEED), m_MouseSensitivity(SENSITIVITY), m_Fov(FOV), m_CameraYaw(yaw), m_CameraPitch(pitch)
    {
        m_Direction = glm::vec3(0.0f, 0.0f, -1.0f);
        m_Position = glm::vec3(posX, posY, posZ);
        m_WorldUp = glm::vec3(upX, upY, upZ);
        UpdateCameraVectors();
    }

    void FlyCamera::ProcessKeyboard(CameraMovement movement, float deltaTime) {
        float velocity = m_MovementSpeed * deltaTime;

        switch (movement)
        {
        case FORWARD:
            m_Position += m_Direction * velocity;
            break;
        case BACKWARD:
            m_Position -= m_Direction * velocity;
            break;
        case RIGHT:
            m_Position += m_CameraRight * velocity;
            break;
        case LEFT:
            m_Position -= m_CameraRight * velocity;
            break;
        case UP:
            m_Position += m_WorldUp * velocity;
            break;
        case DOWN:
            m_Position -= m_WorldUp * velocity;
            break;
        }
    }

    void FlyCamera::ProcessMouseMovement(float offsetX, float offsetY, bool constrainPitch = true) {
        offsetX *= m_MouseSensitivity;
        offsetY *= m_MouseSensitivity;

        m_CameraYaw += offsetX;
        m_CameraPitch += offsetY;

        if (constrainPitch){
            if (m_CameraPitch > 89.0f) {
                m_CameraPitch = 89.0f;
            } 
            if (m_CameraPitch < -89.0f) {
                m_CameraPitch = -89.0f;
            }
        }

        UpdateCameraVectors();
    }

    void FlyCamera::ProcessMouseScroll(float offset) {
        m_Fov -= offset;
        if (m_Fov < 1.0f) {
            m_Fov = 1.0f;
        }
        if (m_Fov > 90.0f) {
            m_Fov = 90.0f;
        }
    }
    
    void FlyCamera::UpdateCamera(const glm::vec3& position, float yaw, float pitch) {
        m_Position = position;
        m_CameraYaw = yaw;
        m_CameraPitch = pitch;

        UpdateCameraVectors();
    }

    void FlyCamera::UpdateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(m_CameraYaw)) * cos(glm::radians(m_CameraPitch));
        front.y = sin(glm::radians(m_CameraPitch));
        front.z = sin(glm::radians(m_CameraYaw)) * cos(glm::radians(m_CameraPitch));
        m_Direction = glm::normalize(front);
        
        m_CameraRight = glm::normalize(glm::cross(m_Direction, m_WorldUp));
        m_CameraUp = glm::normalize(glm::cross(m_CameraRight, m_Direction));

        m_View = glm::lookAt(m_Position, m_Position + m_Direction, m_CameraUp);
    }

    Camera2D::Camera2D(glm::vec3 position, float zoom) {
        UpdateCamera(zoom);
        UpdateCamera(position);
    }

    void Camera2D::UpdateCamera(glm::vec3 position) {
        m_Position = position;
        m_View = glm::scale(glm::mat4(1.0f), glm::vec3(m_Zoom, -m_Zoom, m_Zoom));
        m_View = glm::translate(m_View, glm::vec3(-m_Position.x, -m_Position.y, -m_Position.z));
    }

    void Camera2D::UpdateCamera(float zoom) {
        m_Zoom = zoom;
    }
}