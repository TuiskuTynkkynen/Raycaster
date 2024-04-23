#pragma once

#include <glad/glad.h>
#include  <cstdint>

namespace Core {
	class Texture2D
	{
	private:
		uint32_t m_RendererID;
		bool m_UsesMipMap;
	public:
		Texture2D(GLint wrapS, GLint WrapT, GLint filterMin, GLint filterMax);
		~Texture2D();

		void BindImage(const char* fileName);
		void BindData(const unsigned char* data, uint32_t height, uint32_t width, uint32_t channelCount);
		void Activate(uint32_t unitIndex);
	};
}

