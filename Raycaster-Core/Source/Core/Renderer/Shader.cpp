#include "Shader.h"

#include "Core/Debug/Debug.h"
#include "Platform.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#if !defined(PLATFORM_EMSCRIPTEN)
    #include <glad/gl.h>
#else
    #include <GLES3/gl3.h>
#endif

#include <string>
#include <filesystem> 
#include <fstream>
#include <sstream>

enum class ShaderType {
    NONE = -1, Vertex = 0, Fragment = 1
};

uint32_t CreateShaderProgram(const char* vertexShaderCode, const char* fragmentShaderCode) {
    int32_t compileStatus;
    char shaderCompileLog[512];

    const uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileStatus);
    if (!compileStatus) {
        glGetShaderInfoLog(vertexShader, 512, NULL, shaderCompileLog);
        RC_ERROR("Vertex shader compilation failed\n{}", shaderCompileLog);
    }

    const uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileStatus);
    if (!compileStatus) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, shaderCompileLog);
        RC_ERROR("Fragment shader compilation failed\n{}", shaderCompileLog);
    }

    const uint32_t programID = glCreateProgram();
    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);
    glLinkProgram(programID);

    glGetProgramiv(programID, GL_LINK_STATUS, &compileStatus);
    if (!compileStatus) {
        glGetProgramInfoLog(programID, 512, NULL, shaderCompileLog);
        RC_ERROR("Shader program linking failed\n{}", shaderCompileLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return programID;
}

namespace Core {
    Shader::Shader(std::string_view shaderFilePath)
        : m_RendererID(0)
    {
        std::stringstream shaderStrings[2];
        {
            std::filesystem::path filePath = shaderFilePath;
            if (!filePath.is_absolute()) {
                filePath = ApplicationDirectory() / filePath;
            }

            std::ifstream ShaderFile(filePath);

            if (!ShaderFile.good()) {
                RC_ERROR("Shader file, {}, read failed.", filePath.string());
                return;
            }

            std::string line;
            ShaderType type = ShaderType::NONE;
            while (getline(ShaderFile, line)) {
                if (line.find("#shader") != std::string::npos) {
                    if (line.find("vertex") != std::string::npos) {
                        type = ShaderType::Vertex;
                    }
                    else if (line.find("fragment") != std::string::npos) {
                        type = ShaderType::Fragment;
                    }
                    continue;
                }
                
                if (type == ShaderType::NONE) {
                    RC_ERROR("Shader file, {}, must begin with \"#shader vertex\" or \"#shader fragment\" directive.", filePath.string());
                    return;
                }

                shaderStrings[(int)type] << line << "\n";
            }

            if (shaderStrings[0].tellp() == std::streampos(0)) {
                RC_ERROR("Could not find vertex shader code in file, {}.", filePath.string());
                return;
            }

            if (shaderStrings[1].tellp() == std::streampos(0)) {
                RC_ERROR("Could not find fragment shader code in file, {}.", filePath.string());
                return;
            }
        }
        
        m_RendererID = CreateShaderProgram(shaderStrings[0].str().c_str(), shaderStrings[1].str().c_str());
    }

    Shader::Shader(std::string_view vertexFileName, std::string_view fragmentFileName)
        : m_RendererID(0)
    {
        const std::filesystem::path basePath = ApplicationDirectory();
        std::stringstream vertexCode, fragmentCode;
        
        {
            std::filesystem::path vertexPath = vertexFileName;
            if (!vertexPath.is_absolute()) {
                vertexPath = basePath / vertexPath;
            }


            std::ifstream vertexFile(vertexPath);
            if (!vertexFile.good()) {
                RC_ERROR("Vertex shader file, {}, read failed.", vertexPath.string());
                return;
            }

            vertexCode << vertexFile.rdbuf();
        }
        
        {
            std::filesystem::path fragmentPath = vertexFileName;
            if (!fragmentPath.is_absolute()) {
                fragmentPath = basePath / fragmentPath;
            }


            std::ifstream fragmentFile(fragmentPath);
            if (!fragmentFile.good()) {
                RC_ERROR("Fragment shader file, {}, read failed.", fragmentPath.string());
                return;
            }

            fragmentCode << fragmentFile.rdbuf();
        }

        m_RendererID = CreateShaderProgram(vertexCode.str().c_str(), fragmentCode.str().c_str());
    }

    Shader::Shader(const char* vertexShaderCode, const char* fragmentShaderCode)
        : m_RendererID(CreateShaderProgram(vertexShaderCode, fragmentShaderCode)) {}

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

    void Shader::SetUniformBlockBinding(const char* name, uint32_t bindingIndex){
    uint32_t block = glGetUniformBlockIndex(m_RendererID, name);
        glUniformBlockBinding(m_RendererID, block, bindingIndex);
    }

    uint32_t Shader::GetUniformLocation(const char* name) {
        if (auto iter = m_UniformLocationCache.find(name); iter != m_UniformLocationCache.end()) {
            return iter->second;
        }

        int32_t location = glGetUniformLocation(m_RendererID, name);
        if (location < 0) {
            RC_WARN("Uniform, {}, does not exist", name);
            return 0;
        }

        m_UniformLocationCache.try_emplace(name, location);
        return location;
    }
}
