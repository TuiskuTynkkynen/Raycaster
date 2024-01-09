#include "Core/Core.h"
#include "Core/Shader.h"
#include "Core/Texture.h"
#include "Core/Camera.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <utils/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

uint32_t windowHeigth = 800;
uint32_t windowWidth = 600;

Core::FlyCamera camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
float lastCursorX, lastCursorY;
bool firstMouse = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
    
    windowWidth = width;
    windowHeigth = height;
}

void processInput(GLFWwindow* window, float deltaTime){
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(Core::FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.ProcessKeyboard(Core::BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(Core::LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(Core::RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera.ProcessKeyboard(Core::UP, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera.ProcessKeyboard(Core::DOWN, deltaTime);
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse)
    {
        lastCursorX = xpos;
        lastCursorY = ypos;
        firstMouse = false;
    }

    camera.ProcessMouseMovement(xpos - lastCursorX, lastCursorY - ypos, true);

    lastCursorX = xpos;
    lastCursorY = ypos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void processInput2D(GLFWwindow* window, const uint32_t* map, int height, int width, Core::RaycasterCamera& camera, glm::vec3& playerPosition, float& playerRotation, float deltaTime) {
    float velocity = 2.0f * deltaTime;
    float rotationSpeed = 180.0f * deltaTime;

    glm::vec3 front;
    front.x = cos(glm::radians(playerRotation));
    front.y = -sin(glm::radians(playerRotation)); //player y is flipped (array index)
    front.z = 0.0f;
    
    glm::vec3 oldPosition = playerPosition;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        playerPosition += velocity * front;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        playerPosition -= velocity * front;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        playerRotation += rotationSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        playerRotation -= rotationSpeed;
    }

    if (map[(int)oldPosition.y * width + (int)playerPosition.x] != 0) {
        playerPosition.x = oldPosition.x;
    }
    if (map[(int)playerPosition.y * width + (int)oldPosition.x] != 0) {
        playerPosition.y = oldPosition.y;
    }

    camera.UpdateCamera(playerPosition, playerRotation);
}

void raycast2D(GLFWwindow* window){
    const uint32_t map[]{
        1, 1, 1, 1, 1,
        1, 1, 0, 1, 1,
        1, 0, 0, 0, 1,
        1, 1, 0, 1, 1,
        1, 1, 1, 1, 1,
    };
    
    const uint32_t heigth = 5, width = 5;
    const float centreY = (float)(heigth - 1) / 2, centreX = (float)(width - 1) / 2;
    const uint32_t size = heigth * width;
    const float mapScalingFactor = 1.4f;
    const float centre = sqrt((float)size) / mapScalingFactor;

    //TODO dynamic scaling based on sceen h and w
    const float quadVertices[]{
         0.48f,  0.48f, 0.0f, 
         0.48f, -0.48f, 0.0f,
        -0.48f,  0.48f, 0.0f,
        -0.48f, -0.48f, 0.0f,
    };
    const uint32_t quadIndices[]{
        0, 1, 2,
        1, 2, 3,
    };

    glm::vec3 tilePositions[size];
    for (int i = 0; i < size; i++) {
        uint32_t mapX = i % width;
        uint32_t mapY = i / width;

        float worldX = (mapX - centreX) / (centre);
        float worldY = (centreY - mapY) / (centre);
        float worldZ = 0.0f;
        tilePositions[i] = glm::vec3(worldX, worldY, worldZ);
    }

    uint32_t quadVAO;
    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

    uint32_t quadVBO;
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int quadEBO;
    glGenBuffers(1, &quadEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    const float rayVertices[] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
    };

    uint32_t rayVAO;
    glGenVertexArrays(1, &rayVAO);
    glBindVertexArray(rayVAO);

    uint32_t rayVBO;
    glGenBuffers(1, &rayVBO);
    glBindBuffer(GL_ARRAY_BUFFER, rayVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rayVertices), rayVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    Core::Shader shader("2DVertexShader.glsl", "2DFragmentShader.glsl");
    shader.use();

    glm::vec3 mapScale = glm::vec3(1 / centre, 1 / centre, 1.0f);
    
    glm::vec3 playerPosition = glm::vec3((float)width / 2, (float)heigth / 2, 1.0f);
    glm::vec3 playerScale = mapScale * 0.4f;
    float playerRotation = 90.0f;

    Core::RaycasterCamera camera(playerPosition, playerRotation, centre, width, heigth);

    glm::mat4 transform;

    uint32_t rayCount = 60;
    
    glm::vec3 AxisZ = glm::vec3(0.0f, 0.0f, 1.0f);
    float deltaTime = 0.0f, lastFrame = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput2D(window, map, heigth, width, camera, playerPosition, playerRotation, deltaTime);
        
        glClearColor(0.05f, 0.075f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(quadVAO);
        for (int i = 0; i < size; i++) {
            transform = glm::mat4(1.0f);
            transform = glm::translate(transform, tilePositions[i]);
            transform = glm::translate(transform, -camera.cameraPosition);
            transform = glm::scale(transform, mapScale);
            shader.setMat4("transform", transform);
            shader.setVec3("colour", (map[i] != 0) ? glm::vec3(0.8f) : glm::vec3(0.2f));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        shader.setVec3("colour", glm::vec3(1.0f, 0.0f, 0.0f));
        transform = glm::mat4(1.0f);
        transform = glm::rotate(transform, glm::radians(playerRotation), AxisZ);
        transform = glm::scale(transform, playerScale);
        shader.setMat4("transform", transform);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
       
        glBindVertexArray(rayVAO);
        //The actual raycasting
        for (int i = 0; i < rayCount; i++)
        {
            float cameraX = 2 * i / float(rayCount) - 1;
            glm::vec3 rayDirection = camera.cameraDirection + camera.cameraPlane * cameraX;
            glm::vec3 deltaDistance = glm::abs((float)1 / rayDirection);
            
            uint32_t mapX = (int)playerPosition.x;
            uint32_t mapY = (int)playerPosition.y;

            int32_t stepX = (rayDirection.x > 0) ? 1 : -1;
            int32_t stepY = (rayDirection.y < 0) ? 1 : -1;

            glm::vec3 sideDistance = deltaDistance;
            sideDistance.x *= (rayDirection.x < 0) ? (playerPosition.x - mapX) : (mapX + 1.0f - playerPosition.x);
            sideDistance.y *= (rayDirection.y > 0) ? (playerPosition.y - mapY) : (mapY + 1.0f - playerPosition.y);
            
            uint32_t hit = 0;
            uint32_t side = 0;

            while (hit == 0)
            {
                if (sideDistance.x < sideDistance.y) {
                    sideDistance.x += deltaDistance.x;
                    mapX += stepX;
                    side = 0;
                    shader.setVec3("colour", glm::vec3(0.0f, 0.0f, 1.0f));
                }
                else {
                    sideDistance.y += deltaDistance.y;
                    mapY += stepY;
                    side = 1;
                    shader.setVec3("colour", glm::vec3(0.0f, 1.0f, 0.0f));
                }

                if (mapY >= heigth || mapX >= width) {
                    std::cout << "ERROR: INDEX OUT OF BOUNDS" << std::endl;
                    break;
                }
                if (map[mapY * width + mapX] > 0) {
                    hit = 1;
                }
            }

            float perpWallDist;
            if (side == 0) { 
                perpWallDist = (sideDistance.x - deltaDistance.x); 
            } else { 
                perpWallDist = (sideDistance.y - deltaDistance.y);
            }

            glm::vec3 ray = rayDirection * perpWallDist * mapScale;
            ray.z = 0;

            transform = glm::mat4(1.0f);
            transform = glm::scale(transform, ray);
            shader.setMat4("transform", transform);
            glDrawArrays(GL_LINES, 0, 2);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteBuffers(1, &quadVBO);

    glDeleteVertexArrays(1, &rayVAO);
    glDeleteBuffers(1, &rayVBO);
}

int main(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(windowHeigth, windowWidth, "Raycaster", NULL, NULL);
    
    if (window == NULL){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
	
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    glViewport(0, 0, windowHeigth, windowWidth);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    raycast2D(window);
    return 0;
    
    glEnable(GL_DEPTH_TEST);
    
    float vertices[] = {
        //vertices             normals               texture coordinates
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
                                                    
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
                                                    
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
                                                    
         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
                                                    
        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
                                                    
        -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
    };

    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f,  0.0f,  0.0f),
        glm::vec3(2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f,  2.0f, -2.5f),
        glm::vec3(1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    uint32_t cubeCount = 10;

    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f,  0.2f,  2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3(0.0f,  0.0f, -3.0f)
    };

    uint32_t pointLightCount = 4;

    uint32_t vertexArrayObject;
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    uint32_t vertexBufferObject;
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    uint32_t lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    uint32_t lightVBO;
    glGenBuffers(1, &lightVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
    Core::Shader lightCubeShader("lightVertexShader.glsl", "lightFragmentShader.glsl");
    
    Core::Shader lightingShader("lightingVertexShader.glsl", "lightingFragmentShader.glsl");
    lightingShader.use();
    lightingShader.setInt("NR_POINT_LIGHTS", 4);

    lightingShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
    lightingShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
    lightingShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
    lightingShader.setFloat("material.shininess", 64.0f);

    Core::Texture2D diffuseMap(GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    diffuseMap.BindImage("crate.png");
    Core::Texture2D specularMap(GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    specularMap.BindImage("crate_specular.png");
    
    diffuseMap.Activate(0);
    lightingShader.setInt("material.diffuse", 0);
    specularMap.Activate(1);
    lightingShader.setInt("material.specular", 1);
    
    lightingShader.setVec3("spotLight.position", 0.0f, 0.0f, 0.0f);
    lightingShader.setVec3("spotLight.direction", 0.0f, 0.0f, 1.0f);

    lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(5.0f)));
    lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(10.0f)));
    
    lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    lightingShader.setVec3("spotLight.diffuse", 0.25f, 0.25f, 0.25f); 
    lightingShader.setVec3("spotLight.specular", 0.5f, 0.5f, 0.5f);

    lightingShader.setVec3("dirLight.ambient", 0.2f, 0.2f, 0.3f);
    lightingShader.setVec3("dirLight.diffuse", 0.6f, 0.5f, 0.5f);
    lightingShader.setVec3("dirLight.specular", 0.5f, 0.2f, 0.2f);

    for (int i = 0; i < pointLightCount; i++) {
        std::string lightName = "pointLights[i]";
        lightName[12] = '0' + i;
        
        lightingShader.setVec3(lightName + ".ambient", 0.1f, 0.1f, 0.1f);
        lightingShader.setVec3(lightName + ".diffuse", 0.6f, 0.5f, 0.5f);
        lightingShader.setVec3(lightName + ".specular", 0.75f, 0.75f, 1.0f);
        lightingShader.setFloat(lightName + ".constant", 1.0f);
        lightingShader.setFloat(lightName + ".linear", 0.09f);
        lightingShader.setFloat(lightName + ".quadratic", 0.032f);
    }

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    glm::mat3 normal;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window, deltaTime);

        glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.fov), (float)windowHeigth / (float)windowWidth, 0.1f, 100.0f);

        lightingShader.use();

        lightingShader.setMat4("view", view);
        lightingShader.setMat4("projection", projection);

        glBindVertexArray(vertexArrayObject);
        for (int i = 0; i < cubeCount; i++) {
            float angle = 20.0f * i;

            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

            normal = glm::mat3(glm::transpose(glm::inverse(view * model)));

            lightingShader.setMat3("normalMatrix", normal);
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glm::vec3 direction = view * glm::vec4(-1.0f, -1.0f, -1.0f, 0.0f);
        lightingShader.setVec3("dirLight.direction", direction);

        for (int i = 0; i < pointLightCount; i++) {
            std::string lightName = "pointLights[i]";
            lightName[12] = '0' + i;

            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            direction = view * model * glm::vec4(pointLightPositions[i], 1.0f);
            lightingShader.setVec3(lightName + ".position", direction);
        }

        glBindVertexArray(lightVAO);
        lightCubeShader.use();
        lightCubeShader.setMat4("view", view);
        lightCubeShader.setMat4("projection", projection);
        
        for (int i = 0; i < pointLightCount; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f));

            lightCubeShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vertexArrayObject);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &vertexBufferObject);
    glDeleteBuffers(1, &lightVBO);
    
    glfwTerminate();
    return 0;
}
