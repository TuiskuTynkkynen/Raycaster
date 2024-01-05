#include "Core/Core.h"
#include "Core/Shader.h"
#include "Core/Texture.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <utils/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}

int main(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Raycaster", NULL, NULL);
    
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
    
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    

    //Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    float vertices[] = {
        //positions               //colours    //texture coordinates
        -0.5f,-0.5f, 0.0f, 1.0f,  0.0f, 0.0f,  0.0f, 0.0f,
         0.5f,-0.5f, 0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 0.0f,
        -0.5f, 0.5f, 0.0f, 0.0f,  0.0f, 1.0f,  0.0f, 1.0f,
         0.5f, 0.5f, 0.0f, 1.0f,  1.0f, 1.0f,  1.0f, 1.0f,
    };
    
    uint32_t indices[] = {
        0, 1, 2,
        1, 2, 3,
    };

    uint32_t vertexArrayObject;
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    uint32_t vertexBufferObject;
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    uint32_t elementBufferObject;
    glGenBuffers(1, &elementBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    Core::Texture2D texture0(GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    texture0.BindImage("container.jpg", GL_RGB);

    Core::Texture2D texture1(GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    texture1.BindImage("texture.jpg", GL_RGB);

    texture0.Activate(0);
    texture1.Activate(1);
    
    Core::Shader shader("vertexShader.glsl", "fragmentShader.glsl");
    shader.use();
    shader.setInt("currentTexture0", 0);
    shader.setInt("currentTexture1", 1);

    uint32_t tranformLocation = glGetUniformLocation(shader.ID, "transform");

    int32_t frameNumber = 0;
    while (!glfwWindowShouldClose(window)){
        glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        float time = frameNumber++ / 200.0f;
        float offset = 0.0f;
        offset = (sin(time) + 1.0f) / 10.0f;

        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(0.0f, offset, 0.0f));
        transform = glm::rotate(transform, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(tranformLocation, 1, GL_FALSE, glm::value_ptr(transform));

        shader.use();
        shader.setFloat("uniformOffset", offset);
        glBindVertexArray(vertexArrayObject);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vertexArrayObject);
    glDeleteBuffers(1, &vertexBufferObject);
    glDeleteBuffers(1, &elementBufferObject);
    
    glfwTerminate();
    return 0;
}
