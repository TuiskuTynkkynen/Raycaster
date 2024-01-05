#include <Core/Camera.h>

#include <glm/gtc/matrix_transform.hpp>

namespace Core {
    // Default camera values
    const float YAW = -90.0f;
    const float PITCH = 0.0f;
    const float SPEED = 5.0f;
    const float SENSITIVITY = 0.1f;
    const float FOV = 45.0f;

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
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
        front.y = sin(glm::radians(cameraPitch));
        front.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
        cameraFront = glm::normalize(front);
        
        cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
        cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
    }
}