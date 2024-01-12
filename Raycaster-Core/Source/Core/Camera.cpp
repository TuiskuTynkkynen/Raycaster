#include <Core/Camera.h>

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 5.0f;
const float SENSITIVITY = 0.1f;
const float FOV = 45.0f; 

const glm::vec3 worldUp2D = glm::vec3(0.0f, 0.0f, 1.0f);

namespace Core {

    FlyCamera::FlyCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH)
        : position(position), m_movementSpeed(SPEED), m_mouseSensitivity(SENSITIVITY), fov(FOV), m_worldUp(up), m_cameraYaw(yaw), m_cameraPitch(pitch)
    {
       direction = glm::vec3(0.0f, 0.0f, -1.0f);
       updateCameraVectors();
    }

    FlyCamera::FlyCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
        : m_movementSpeed(SPEED), m_mouseSensitivity(SENSITIVITY), fov(FOV), m_cameraYaw(yaw), m_cameraPitch(pitch)
    {
        direction = glm::vec3(0.0f, 0.0f, -1.0f);
        position = glm::vec3(posX, posY, posZ);
        m_worldUp = glm::vec3(upX, upY, upZ);
        updateCameraVectors();
    }

    glm::mat4 FlyCamera::GetViewMatrix()
    {
        return glm::lookAt(position, position + direction, m_cameraUp);
    }

    void FlyCamera::ProcessKeyboard(CameraMovement movement, float deltaTime)
    {
        float velocity = m_movementSpeed * deltaTime;

        switch (movement)
        {
        case FORWARD:
            position += direction * velocity;
            break;
        case BACKWARD:
            position -= direction * velocity;
            break;
        case RIGHT:
            position += m_cameraRight * velocity;
            break;
        case LEFT:
            position -= m_cameraRight * velocity;
            break;
        case UP:
            position += m_worldUp * velocity;
            break;
        case DOWN:
            position -= m_worldUp * velocity;
            break;
        }
    }

    void FlyCamera::ProcessMouseMovement(float offsetX, float offsetY, bool constrainPitch = true)
    {
        offsetX *= m_mouseSensitivity;
        offsetY *= m_mouseSensitivity;

        m_cameraYaw += offsetX;
        m_cameraPitch += offsetY;

        if (constrainPitch){
            if (m_cameraPitch > 89.0f) {
                m_cameraPitch = 89.0f;
            } 
            if (m_cameraPitch < -89.0f) {
                m_cameraPitch = -89.0f;
            }
        }

        updateCameraVectors();
    }

    void FlyCamera::ProcessMouseScroll(float offset)
    {
        fov -= offset;
        if (fov < 1.0f) {
            fov = 1.0f;
        }
        if (fov > 90.0f) {
            fov = 90.0f;
        }
    }

    void FlyCamera::updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(m_cameraYaw)) * cos(glm::radians(m_cameraPitch));
        front.y = sin(glm::radians(m_cameraPitch));
        front.z = sin(glm::radians(m_cameraYaw)) * cos(glm::radians(m_cameraPitch));
        direction = glm::normalize(front);
        
        m_cameraRight = glm::normalize(glm::cross(direction, m_worldUp));
        m_cameraUp = glm::normalize(glm::cross(m_cameraRight, direction));
    }


    RaycasterCamera::RaycasterCamera(glm::vec3 playerPosition, float yaw, float centre, float width, float height) 
        : m_reciprocalCentre(1 / centre), m_halfWidth(width/2), m_halfHeight(height/2), m_cameraYaw(yaw)
    {
        position = glm::vec3(0.0f);
        position.x = (playerPosition.x - m_halfWidth) * m_reciprocalCentre;
        position.y = (m_halfHeight - playerPosition.y) * m_reciprocalCentre;

        glm::vec3 front(0.0f);
        front.x = cos(glm::radians(m_cameraYaw));
        front.y = sin(glm::radians(m_cameraYaw));

        direction = glm::normalize(front);
        plane = glm::normalize(glm::cross(direction, worldUp2D));
    }

    RaycasterCamera::RaycasterCamera(float posX, float posY, float yaw, float centre, float width, float height) 
        : m_reciprocalCentre(1 / centre), m_halfWidth(width/2), m_halfHeight(height/2), m_cameraYaw(yaw)
    {
        position = glm::vec3(0.0f);
        position.x = (posX - m_halfWidth) * m_reciprocalCentre;
        position.y = (m_halfHeight - posY) * m_reciprocalCentre;

        glm::vec3 front(0.0f);
        front.x = cos(glm::radians(m_cameraYaw));
        front.y = sin(glm::radians(m_cameraYaw));

        direction = glm::normalize(front);
        plane = glm::normalize(glm::cross(direction, worldUp2D));
    }

    void RaycasterCamera::UpdateCamera(const glm::vec3& playerPosition, float yaw) {
        position.x = (playerPosition.x - m_halfWidth) * m_reciprocalCentre;
        position.y = (m_halfHeight - playerPosition.y) * m_reciprocalCentre;

        m_cameraYaw = yaw;
        glm::vec3 front(0.0f);
        front.x = cos(glm::radians(m_cameraYaw));
        front.y = sin(glm::radians(m_cameraYaw));

        direction = glm::normalize(front);
        plane = glm::normalize(glm::cross(direction, worldUp2D));
    }

    void RaycasterCamera::UpdateCamera(float posX, float posY, float yaw) {
        position.x = (posX - m_halfWidth) * m_reciprocalCentre;
        position.y = (m_halfHeight - posY) * m_reciprocalCentre;

        m_cameraYaw = yaw;
        glm::vec3 front(0.0f);
        front.x = cos(glm::radians(m_cameraYaw));
        front.y = sin(glm::radians(m_cameraYaw));

        direction = glm::normalize(front);
        plane = glm::normalize(glm::cross(direction, worldUp2D));
    }
}