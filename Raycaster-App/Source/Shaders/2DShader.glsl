#shader vertex

#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 transform;

void main()
{
   gl_Position = transform * vec4(aPos, 1.0f);
}


#shader fragment

#version 330 core
out vec4 FragColor;

uniform vec3 colour;

void main(){
	FragColor = vec4(colour, 1.0);
}