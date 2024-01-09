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
        : movementSpeed(SPEED), mouseSensitivity(SENSITIVITY) 
    {
        cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
        fov = FOV;
        cameraPosition = position;
        worldUp = up;
        cameraYaw = yaw;
        cameraPitch = pitch;
        updateCameraVectors();
    }

    FlyCamera::FlyCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
        : movementSpeed(SPEED), mouseSensitivity(SENSITIVITY)
    {
        cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
        fov = FOV;
        cameraPosition = glm::vec3(posX, posY, posZ);
        worldUp = glm::vec3(upX, upY, upZ);
        cameraYaw = yaw;
        cameraPitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 FlyCamera::GetViewMatrix()
    {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
    }

    void FlyCamera::ProcessKeyboard(CameraMovement direction, float deltaTime)
    {
        float velocity = movementSpeed * deltaTime;

        switch (direction)
        {
        case FORWARD:
            cameraPosition += cameraFront * velocity;
            break;
        case BACKWARD:
            cameraPosition -= cameraFront * velocity;
            break;
        case RIGHT:
            cameraPosition += cameraRight * velocity;
            break;
        case LEFT:
            cameraPosition -= cameraRight * velocity;
            break;
        case UP:
            cameraPosition += worldUp * velocity;
            break;
        case DOWN:
            cameraPosition -= worldUp * velocity;
            break;
        }
    }

    void FlyCamera::ProcessMouseMovement(float offsetX, float offsetY, bool constrainPitch = true)
    {
        offsetX *= mouseSensitivity;
        offsetY *= mouseSensitivity;

        cameraYaw += offsetX;
        cameraPitch += offsetY;

        if (constrainPitch){
            if (cameraPitch > 89.0f) {
                cameraPitch = 89.0f;
            } 
            if (cameraPitch < -89.0f) {
                cameraPitch = -89.0f;
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
        front.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
        front.y = sin(glm::radians(cameraPitch));
        front.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
        cameraFront = glm::normalize(front);
        
        cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
        cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
    }


    RaycasterCamera::RaycasterCamera(glm::vec3 playerPosition, float yaw, float centre, float width, float height) 
        : reciprocalCentre(1 / centre), halfWidth(width/2), halfHeight(height/2)
    {
        cameraPosition = glm::vec3(-0.5f);
        cameraPosition.x = (playerPosition.x - halfWidth) * reciprocalCentre;
        cameraPosition.y = (halfHeight - playerPosition.y) * reciprocalCentre;

        cameraYaw = yaw;

        glm::vec3 front(0.0f);
        front.x = cos(glm::radians(cameraYaw));
        front.y = sin(glm::radians(cameraYaw));

        cameraDirection = glm::normalize(front);
        cameraPlane = glm::normalize(glm::cross(cameraDirection, worldUp2D));
    }

    RaycasterCamera::RaycasterCamera(float posX, float posY, float yaw, float centre, float width, float height) 
        : reciprocalCentre(1 / centre), halfWidth(width/2), halfHeight(height/2)
    {
        cameraPosition = glm::vec3(0.0f);
        cameraPosition.x = (posX - halfWidth) * reciprocalCentre;
        cameraPosition.y = (halfHeight - posY) * reciprocalCentre;

        cameraYaw = yaw;

        glm::vec3 front(0.0f);
        front.x = cos(glm::radians(cameraYaw));
        front.y = sin(glm::radians(cameraYaw));

        cameraDirection = glm::normalize(front);
        cameraPlane = glm::normalize(glm::cross(cameraDirection, worldUp2D));
    }

    void RaycasterCamera::UpdateCamera(const glm::vec3& playerPosition, float yaw) {
        cameraPosition.x = (playerPosition.x - halfHeight) * reciprocalCentre;
        cameraPosition.y = (halfWidth - playerPosition.y) * reciprocalCentre;

        cameraYaw = yaw;
        glm::vec3 front(0.0f);
        front.x = cos(glm::radians(cameraYaw));
        front.y = sin(glm::radians(cameraYaw));

        cameraDirection = glm::normalize(front);
        cameraPlane = glm::normalize(glm::cross(cameraDirection, worldUp2D));
    }

    void RaycasterCamera::UpdateCamera(float posX, float posY, float yaw) {
        cameraPosition.x = (posX - halfHeight) * reciprocalCentre;
        cameraPosition.y = (halfWidth - posY) * reciprocalCentre;

        cameraYaw = yaw;
        glm::vec3 front(0.0f);
        front.x = cos(glm::radians(cameraYaw));
        front.y = sin(glm::radians(cameraYaw));

        cameraDirection = glm::normalize(front);
        cameraPlane = glm::normalize(glm::cross(cameraDirection, worldUp2D));
    }
}