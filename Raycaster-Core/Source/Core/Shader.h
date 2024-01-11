#pragma once

#include <glm/glm.hpp>

#include <string>

namespace Core {
	class Shader
	{
    private:
    	uint32_t m_RendererID;
    public:
        Shader(const char* vertexFileName, const char* fragmentFileName);
        ~Shader();
        void use();
        void setBool(const std::string& name, bool value) const;
        void setInt(const std::string& name, int value) const;
        void setFloat(const std::string& name, float value) const;
        void setVec3(const std::string& name, float valueX, float valueY, float valueZ) const;
        void setVec3(const std::string& name, const glm::vec3& value) const;
        void setVec4(const std::string& name, float valueX, float valueY, float valueZ, float valueW) const;
        void setVec4(const std::string& name, const glm::vec4& value) const;
        void setMat3(const std::string& name, const glm::mat3& value) const;
        void setMat4(const std::string& name, const glm::mat4& value) const;
	};
}

