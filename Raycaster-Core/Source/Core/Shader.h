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
        std::unordered_map<std::string, uint32_t> m_UniformLocationCache;
        uint32_t GetUniformLocation(const std::string& name);
    public:
        Shader(const std::string& shaderFileName);
        Shader(const char* vertexFileName, const char* fragmentFileName);
        ~Shader();

        void Bind();
        void Unbind();
        
        void setBool(const std::string& name, bool value);
        void setInt(const std::string& name, int value);
        void setFloat(const std::string& name, float value);
        void setVec3(const std::string& name, float valueX, float valueY, float valueZ);
        void setVec3(const std::string& name, const glm::vec3& value);
        void setVec4(const std::string& name, float valueX, float valueY, float valueZ, float valueW);
        void setVec4(const std::string& name, const glm::vec4& value);
        void setMat3(const std::string& name, const glm::mat3& value);
        void setMat4(const std::string& name, const glm::mat4& value);
	};
}

