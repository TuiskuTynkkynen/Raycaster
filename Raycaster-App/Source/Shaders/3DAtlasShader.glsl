#shader vertex

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexPos;

out vec2 TexCoords;
out vec3 WorldPosition;

uniform mat4 ViewProjection;
uniform mat4 ModelTransform;

void main()
{
   gl_Position = ViewProjection * ModelTransform * vec4(aPos, 1.0f);
   WorldPosition = (ModelTransform * vec4(aPos, 1.0f)).xyz;
   TexCoords = aTexPos;
}


#shader fragment

#version 330 core
in vec2 TexCoords;
in vec3 WorldPosition;

uniform vec2 AtlasSize;
uniform vec2 AtlasOffset;
uniform vec2 FlipTexture;

uniform vec3 ModelTint;
uniform sampler2D Texture;

#define MAX_POINT_LIGHTS 10
uniform vec3 PointLights[MAX_POINT_LIGHTS];
uniform int LightCount;

out vec4 FragColor;

void main(){
	vec2 uv = (abs(FlipTexture - fract(TexCoords)) + AtlasOffset) / AtlasSize;
    FragColor = texture(Texture, uv);
    
    if(FragColor.a == 0.0f){
        discard;
    }

    float brightness;
    for(int i = 0; i < LightCount; i++){
            float distance = length(WorldPosition - PointLights[i]);
            brightness += 1.0f / (0.95f + 0.1f * distance + 0.03f * (distance * distance));
    }

	FragColor *= vec4(ModelTint * brightness, 1.0f);
}