#shader vertex

#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexPos;

out vec2 TexCoords;
out vec3 WorldPosition;
out vec3 Normal;

uniform mat4 ViewProjection;
uniform mat4 ModelTransform;

void main()
{
    gl_Position = ViewProjection * ModelTransform * vec4(aPos, 1.0f);
    WorldPosition = (ModelTransform * vec4(aPos, 1.0f)).xyz;
    TexCoords = aTexPos;
    Normal = aNormal;
}


#shader fragment

#version 460 core
in vec2 TexCoords;
in vec3 WorldPosition;
in vec3 Normal;

uniform vec2 AtlasSize;
uniform vec2 AtlasOffset;
uniform vec2 FlipTexture;

uniform vec3 ModelTint;
uniform sampler2D Texture;
uniform sampler2D MapTexture;

#define MAX_POINT_LIGHTS 10
uniform vec3 PointLights[MAX_POINT_LIGHTS];
uniform int LightCount;

out vec4 FragColor;

bool trace(vec2 position, vec2 target) {
    vec2 rayDirection = target - position;
    vec2 deltaDistance = abs(1.0f / normalize(rayDirection));

    int mapX = int(position.x);
    int mapY = int(position.y);

    int stepX = (rayDirection.x > 0.0f) ? 1 : -1;
    int stepY = (rayDirection.y > 0.0f) ? 1 : -1;

    vec2 sideDistance = deltaDistance;
    sideDistance.x *= (rayDirection.x < 0.0f) ? (position.x - float(mapX)) : (float(mapX) + 1.0f - position.x);
    sideDistance.y *= (rayDirection.y < 0.0f) ? (position.y - float(mapY)) : (float(mapY) + 1.0f - position.y);
    
    while (mapX != int(target.x) || mapY != int(target.y)) {
        int map = int(texelFetch(MapTexture, ivec2(mapX, mapY), 0).r * 255.0);
        if (mapX < 0 || mapX >= 24 || mapY < 0 || mapY >= 24 || map != 0) {
            return false;
        }

        if (sideDistance.x < sideDistance.y) {
            sideDistance.x += deltaDistance.x;
            mapX += stepX;
        } else {
            sideDistance.y += deltaDistance.y;
            mapY += stepY;
        }
    }

    return true;
}

void main(){
    vec2 uv = (abs(FlipTexture - fract(TexCoords)) + AtlasOffset) / AtlasSize;
    FragColor = texture(Texture, uv);
    
    if(FragColor.a == 0.0f){
        discard;
    }

    float brightness = 0.1;
    for(int i = 0; i < LightCount; i++) {
        float count = 0;

        vec2 pos = WorldPosition.xz + vec2(0.075 * Normal.xz);
        // PCF?? 
        for(int j = 0; j < 4; j++) {
            vec2 offset = vec2(float(j < 2) * 0.05, float(j >= 2) * 0.05);
            offset *= (j % 2 == 0) ? 1.0 : -1.0;
            count += float(trace(pos + offset, PointLights[i].xz));
        }
               
        float distance = length(WorldPosition - PointLights[i]);
        brightness += count / (0.95 + 0.1 * distance + 0.03 * (distance * distance)) / 4.0;
    }

    FragColor *= vec4(ModelTint * brightness, 1.0);
}
