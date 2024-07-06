#shader vertex

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexPos;

out vec2 TexCoords;
uniform mat4 ViewProjection;
uniform mat4 Transform;

uniform mat3 TexTransform;

void main()
{
   gl_Position = ViewProjection * Transform * vec4(aPos, 1.0f);
   TexCoords = (TexTransform * vec3(aTexPos, 1.0f)).xy;
}


#shader fragment

#version 330 core
in vec2 TexCoords;

uniform vec2 AtlasSize;
uniform vec2 AtlasOffset;
uniform int TextureIndex;

uniform vec3 Colour;
uniform sampler2D Textures[2];

out vec4 FragColor;

void main(){
	vec2 uv = (fract(TexCoords) + AtlasOffset) / AtlasSize;
	FragColor = texture(Textures[TextureIndex], uv) * vec4(Colour, 1.0f);
}