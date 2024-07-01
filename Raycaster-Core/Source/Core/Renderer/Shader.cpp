#include "Shader.h"

#include "Core/Debug/Log.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <filesystem> 
#include <fstream>
#include <sstream>

namespace Core {
    Shader::Shader(const std::string& shaderFileName)
        : m_RendererID(0)
    {
        std::string vertexCode, fragmentCode;
        {
            std::filesystem::path filePath = std::filesystem::current_path() / "Source" / "Shaders" / shaderFileName;
            std::ifstream ShaderFile(filePath);

            std::stringstream shaderStrings[2];

            std::string line;
            ShaderType type = ShaderType::NONE;
            while (getline(ShaderFile, line))
            {
                if (line.find("#shader") != std::string::npos) {
                    if (line.find("vertex") != std::string::npos) {
                        type = ShaderType::Vertex;
                    }
                    else if (line.find("fragment") != std::string::npos) {
                        type = ShaderType::Fragment;
                    }
                    continue;
                }
                shaderStrings[(int)type] << line << "\n";
            }

            vertexCode = shaderStrings[0].str();
            fragmentCode = shaderStrings[1].str();
        }

        const char* vertexShaderCode = vertexCode.c_str();
        const char* fragmentShaderCode =fragmentCode.c_str();

        int32_t compileStatus;
        char shaderCompileLog[512];

        uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
        glCompileShader(vertexShader);

        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileStatus);
        if (!compileStatus) {
            glGetShaderInfoLog(vertexShader, 512, NULL, shaderCompileLog);
            RC_ERROR("VERTEX SHADER COMPILATION FAILED\n{}", shaderCompileLog);
        }

        uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileStatus);
        if (!compileStatus) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, shaderCompileLog);
            RC_ERROR("FRAGMENT SHADER COMPILATION FAILED\n{}", shaderCompileLog);
        }

        m_RendererID = glCreateProgram();
        glAttachShader(m_RendererID, vertexShader);
        glAttachShader(m_RendererID, fragmentShader);
        glLinkProgram(m_RendererID);

        glGetProgramiv(m_RendererID, GL_LINK_STATUS, &compileStatus);
        if (!compileStatus) {
            glGetProgramInfoLog(m_RendererID, 512, NULL, shaderCompileLog);
            RC_ERROR("VERTEX SHADER PROGRAM LINKING FAILED\n{}", shaderCompileLog);
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    Shader::Shader(const char* vertexFileName, const char* fragmentFileName)
        : m_RendererID(0)
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
            RC_ERROR("SHADER FILE {} OR {} READ FAILED", vertexFileName, fragmentFileName);
            return;
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
            RC_ERROR("VERTEX SHADER COMPILATION FAILED\n{}", shaderCompileLog);
        }

        uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileStatus);
        if (!compileStatus) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, shaderCompileLog);
            RC_ERROR("FRAGMENT SHADER COMPILATION FAILED\n{}", shaderCompileLog);
        }

        m_RendererID = glCreateProgram();
        glAttachShader(m_RendererID, vertexShader);
        glAttachShader(m_RendererID, fragmentShader);
        glLinkProgram(m_RendererID);

        glGetProgramiv(m_RendererID, GL_LINK_STATUS, &compileStatus);
        if (!compileStatus) {
            glGetProgramInfoLog(m_RendererID, 512, NULL, shaderCompileLog);
            RC_ERROR("VERTEX SHADER PROGRAM LINKING FAILED\n{}", shaderCompileLog);
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    Shader::~Shader() {
        glDeleteProgram(m_RendererID);
    }

    void Shader::Bind()
    {
        glUseProgram(m_RendererID);
    }

    void Shader::Unbind()
    {
        glUseProgram(0);
    }

    void Shader::setBool(const char* name, bool value)
    {
        glUniform1i(GetUniformLocation(name), (int)value);
    }

    void Shader::setInt(const char* name, int value)
    {
        glUniform1i(GetUniformLocation(name), value);
    }

    void Shader::setFloat(const char* name, float value) 
    {
        glUniform1f(GetUniformLocation(name), value);
    }

    void Shader::setVec2(const char* name, const glm::vec2& value) {
        glUniform2f(GetUniformLocation(name), value.x, value.y);
    }

    void Shader::setVec3(const char* name, float valueX, float valueY, float valueZ) {
        glUniform3f(GetUniformLocation(name), valueX, valueY, valueZ);
    }
    
    void Shader::setVec3(const char* name, const glm::vec3& value) {
        glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
    }

    void Shader::setVec4(const char* name, float valueX, float valueY, float valueZ, float valueW) {
        glUniform4f(GetUniformLocation(name), valueX, valueY, valueZ, valueW);
    }
    
    void Shader::setVec4(const char* name, const glm::vec4& value) {
        glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
    }

    void Shader::setMat3(const char* name, const glm::mat3& value) {
        glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
    }

    void Shader::setMat4(const char* name, const glm::mat4& value) {
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
    }

    uint32_t Shader::GetUniformLocation(const char* name) {
        if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) {
            return m_UniformLocationCache[name];
        }

        int32_t location = glGetUniformLocation(m_RendererID, name);
    
        if (location == -1) {
            RC_WARN("UNIFORM {} DOES NOT EXIST", name);
            return 0;
        }
        return location;
    }
}