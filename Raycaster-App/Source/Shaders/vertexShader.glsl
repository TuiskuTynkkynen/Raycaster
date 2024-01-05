#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColour;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 transform;

out vec3 vertexColour;
out vec2 textureCoordinates;

void main()
{
   gl_Position = transform *  vec4(aPos, 1.0f);
   vertexColour = aColour;
   textureCoordinates = aTexCoord;
}