#pragma once

#include <glm/glm.hpp>

#include <string>
#include <unordered_map>

enum class ShaderType {
    NONE = -1, Vertex = 0, Fragment = 1
};

namespace Core {
	class Shader
	{
    private:
    	uint32_t m_RendererID;
        
        //const char* doesn't do extra allocations -> faster than string
        std::unordered_map<const char*, uint32_t> m_UniformLocationCache; 

        uint32_t GetUniformLocation(const char* name);
    public:
        Shader(const std::string& shaderFileName);
        Shader(const char* vertexFileName, const char* fragmentFileName);
        ~Shader();

        void Bind();
        void Unbind();
        
        void setBool(const char* name, bool value);
        void setInt(const char* name, int value);
        void setFloat(const char* name, float value);
        void setVec2(const char* name, const glm::vec2& value);
        void setVec3(const char* name, float valueX, float valueY, float valueZ);
        void setVec3(const char* name, const glm::vec3& value);
        void setVec4(const char* name, float valueX, float valueY, float valueZ, float valueW);
        void setVec4(const char* name, const glm::vec4& value);
        void setMat3(const char* name, const glm::mat3& value);
        void setMat4(const char* name, const glm::mat4& value);
	};
}

