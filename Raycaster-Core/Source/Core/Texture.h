#pragma once

#include <glad/glad.h>
#include  <cstdint>

namespace Core {
	class Texture2D
	{
	private:
		uint32_t ID;
		bool usesMipMap;
	public:
		Texture2D(GLint wrapS, GLint WrapT, GLint filterMin, GLint filterMax);

		void BindImage(const char* fileName, GLenum colourSpace);

		void Activate(uint32_t unitIndex);
	};
}

