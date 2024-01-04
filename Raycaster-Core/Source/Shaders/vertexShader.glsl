#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColour;
layout (location = 2) in vec2 aTexCoord;

uniform float uniformOffset;

out vec3 vertexColour;
out vec2 textureCoordinates;

void main()
{
   gl_Position = vec4(aPos.x, aPos.y + uniformOffset, aPos.z, 1.0f);
   vertexColour = aColour;
   textureCoordinates = aTexCoord;
}