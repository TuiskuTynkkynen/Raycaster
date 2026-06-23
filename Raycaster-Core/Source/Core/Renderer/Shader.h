#pragma once

#include <glm/glm.hpp>

#include <string>
#include <string_view>
#include <unordered_map>

namespace Core {
	class Shader {
    public:
        Shader(std::string_view shaderFileName);
        Shader(std::string_view vertexFileName, std::string_view fragmentFileName);
        Shader(const char* vertexShader, const char* fragmentShader);
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

        void SetUniformBlockBinding(const char* name, uint32_t bindingIndex);
    private:
        uint32_t m_RendererID;
        std::unordered_map<std::string, uint32_t> m_UniformLocationCache;

        uint32_t GetUniformLocation(const char* name);
	};
}

