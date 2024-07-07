#shader vertex

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColour;

uniform mat4 ViewProjection;

out vec3 VertexColour;

void main()
{
   gl_Position = ViewProjection * vec4(aPos, 1.0f);
   VertexColour = aColour;
}


#shader fragment

#version 330 core

in vec3 VertexColour;

out vec4 FragColor;

void main(){
	FragColor = vec4(VertexColour, 1.0f);
}