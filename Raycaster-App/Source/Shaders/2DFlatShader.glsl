#shader vertex

#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 viewProjection;
uniform mat4 transform;

out vec2 TexCoords;

void main()
{
   gl_Position = viewProjection * transform * vec4(aPos, 1.0f);
}


#shader fragment

#version 330 core

uniform vec3 colour;

out vec4 FragColor;
void main(){
	FragColor = vec4(colour, 1.0f);
}