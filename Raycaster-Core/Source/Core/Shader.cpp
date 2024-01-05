#include "Shader.h"

#include <glad/glad.h>

#include <string>
#include <filesystem> 
#include <fstream>
#include <sstream>
#include <iostream>

namespace Core {
    Shader::Shader(const char* vertexFileName, const char* fragmentFileName)
    {
        std::filesystem::path filePath = std::filesystem::current_path() / "Source" / "Shaders";
        
        std::string vertexCode, fragmentCode;
        std::ifstream vertexShaderFile, fragmentShaderFile;

        vertexShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fragmentShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            vertexShaderFile.open(filePath / vertexFileName);
            fragmentShaderFile.open(filePath / fragmentFileName);
         
            std::stringstream vShaderStream, fShaderStream;
            vShaderStream << vertexShaderFile.rdbuf();
            fShaderStream << fragmentShaderFile.rdbuf();
            
            vertexShaderFile.close();
            fragmentShaderFile.close();
            
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure e)
        {
            std::cout << "ERROR SHADER FILE READ FAILED" << std::endl;
        }

        const char* vertexShaderCode = vertexCode.c_str();
        const char* fragmentShaderCode = fragmentCode.c_str();

        int32_t compileStatus;
        char shaderCompileLog[512];

        uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
        glCompileShader(vertexShader);

        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileStatus);
        if (!compileStatus) {
            glGetShaderInfoLog(vertexShader, 512, NULL, shaderCompileLog);
            std::cout << "ERROR: VERTEX SHADER COMPILATION FAILED\n" << shaderCompileLog << std::endl;
        }

        uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileStatus);
        if (!compileStatus) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, shaderCompileLog);
            std::cout << "ERROR: FRAGMENT SHADER COMPILATION FAILED\n" << shaderCompileLog << std::endl;
        }

        ID = glCreateProgram();
        glAttachShader(ID, vertexShader);
        glAttachShader(ID, fragmentShader);
        glLinkProgram(ID);

        glGetProgramiv(ID, GL_LINK_STATUS, &compileStatus);
        if (!compileStatus) {
            glGetProgramInfoLog(ID, 512, NULL, shaderCompileLog);
            std::cout << "ERROR: SHADER PROGRAM LINKING FAILED\n" << shaderCompileLog << std::endl;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    Shader::~Shader() {
        glDeleteProgram(ID);
    }

    void Shader::use()
    {
        glUseProgram(ID);
    }

    void Shader::setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }

    void Shader::setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void Shader::setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
}