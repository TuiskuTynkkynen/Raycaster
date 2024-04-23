#shader vertex

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexPos;

out vec2 TexCoords;

uniform mat4 viewProjection;
uniform mat4 transform;
uniform mat3 texTransform;

void main()
{
   gl_Position = viewProjection * transform * vec4(aPos, 1.0f);
   TexCoords = (texTransform * vec3(aTexPos, 1.0f)).xy;
}


#shader fragment

#version 330 core
in vec2 TexCoords;

uniform vec3 colour;
uniform sampler2D text;

out vec4 FragColor;

void main(){
	FragColor = vec4(colour, texture(text, TexCoords).r);
}