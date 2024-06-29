#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

const glm::vec3 worldUp2D = glm::vec3(0.0f, 0.0f, 1.0f);

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

    glm::mat4 FlyCamera::GetViewMatrix() const {
        return glm::lookAt(m_Position, m_Position + m_Direction, m_CameraUp);
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
    }


    
    RaycasterCamera::RaycasterCamera(const glm::vec3& playerPosition, float yaw, float centre, float width, float height) 
        : m_ReciprocalCentre(1 / centre), m_HalfWidth(width/2), m_HalfHeight(height/2), m_CameraYaw(yaw)
    {
        m_Position = glm::vec3(0.0f);
        m_Position.x = (playerPosition.x - m_HalfWidth) * m_ReciprocalCentre;
        m_Position.y = (m_HalfHeight - playerPosition.y) * m_ReciprocalCentre;

        glm::vec3 front(0.0f);
        front.x = cos(glm::radians(m_CameraYaw));
        front.y = sin(glm::radians(m_CameraYaw));

        m_Direction = glm::normalize(front);
        m_Plane = glm::normalize(glm::cross(m_Direction, worldUp2D));
    }

    RaycasterCamera::RaycasterCamera(float posX, float posY, float yaw, float centre, float width, float height) 
        : m_ReciprocalCentre(1 / centre), m_HalfWidth(width/2), m_HalfHeight(height/2), m_CameraYaw(yaw)
    {
        m_Position = glm::vec3(0.0f);
        m_Position.x = (posX - m_HalfWidth) * m_ReciprocalCentre;
        m_Position.y = (m_HalfHeight - posY) * m_ReciprocalCentre;

        glm::vec3 front(0.0f);
        front.x = cos(glm::radians(m_CameraYaw));
        front.y = sin(glm::radians(m_CameraYaw));

        m_Direction = glm::normalize(front);
        m_Plane = glm::normalize(glm::cross(m_Direction, worldUp2D));
    }

    glm::mat4 RaycasterCamera::GetViewMatrix() const {
        return glm::translate(glm::mat4(1.0f), -m_Position);
    }

    void RaycasterCamera::UpdateCamera(const glm::vec3& playerPosition, float yaw) {
        m_Position.x = (playerPosition.x - m_HalfWidth) * m_ReciprocalCentre;
        m_Position.y = (m_HalfHeight - playerPosition.y) * m_ReciprocalCentre;

        m_CameraYaw = yaw;
        glm::vec3 front(0.0f);
        front.x = cos(glm::radians(m_CameraYaw));
        front.y = sin(glm::radians(m_CameraYaw));

        m_Direction = glm::normalize(front);
        m_Plane = glm::normalize(glm::cross(m_Direction, worldUp2D));
    }

    void RaycasterCamera::UpdateCamera(float posX, float posY, float yaw) {
        m_Position.x = (posX - m_HalfWidth) * m_ReciprocalCentre;
        m_Position.y = (m_HalfHeight - posY) * m_ReciprocalCentre;

        m_CameraYaw = yaw;
        glm::vec3 front(0.0f);
        front.x = cos(glm::radians(m_CameraYaw));
        front.y = sin(glm::radians(m_CameraYaw));

        m_Direction = glm::normalize(front);
        m_Plane = glm::normalize(glm::cross(m_Direction, worldUp2D));
    }
}