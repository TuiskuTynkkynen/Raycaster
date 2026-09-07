#pragma once

namespace Core::ShaderCore {
    inline const char* Simple2DVertexShader = R"(#version 300 es
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColour;
uniform mat4 ViewProjection;
out vec3 VertexColour;

void main() {
     gl_Position = ViewProjection * vec4(aPos, 1.0f);
     VertexColour = aColour;
})";

    inline const char* Simple2DFragmentShader = R"(#version 300 es
in mediump vec3 VertexColour;
out mediump vec4 FragColor;

void main() {
    FragColor = vec4(VertexColour, 1.0f);
})";

    inline const char* Default2DVertexShader = R"(#version 300 es
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColour;
layout (location = 2) in highp vec2 aTexPos;
layout (location = 3) in highp vec2 aAtlasOffset;
layout (location = 4) in float aTexIndex;
uniform mat4 ViewProjection;
out vec4 VertexColour;
out highp vec2 TexCoords, AtlasOffset;
flat out int TextureIndex;

void main() {
    gl_Position = ViewProjection * vec4(aPos, 1.0f);
    VertexColour = aColour;
    TexCoords = aTexPos;
    AtlasOffset = aAtlasOffset;
    AtlasOffset += 1e-5;
    TextureIndex = int(aTexIndex);
})";

    inline const char* Default2DFragmentShader = R"(#version 300 es
precision mediump float;
in vec4 VertexColour;
in highp vec2 TexCoords, AtlasOffset;
flat in int TextureIndex;
uniform sampler2D Textures[16];
uniform highp vec2 AtlasSize;
out vec4 FragColor;

vec4 sampleTexture(int index, vec2 uv) { // GOD I LOVE version 300 es
    switch(index) {
        case 0: return texture(Textures[0], uv);
        case 1: return texture(Textures[1], uv);
        case 2: return texture(Textures[2], uv);
        case 3: return texture(Textures[3], uv);
        case 4: return texture(Textures[4], uv);
        case 5: return texture(Textures[5], uv);
        case 6: return texture(Textures[6], uv);
        case 7: return texture(Textures[7], uv);
        case 8: return texture(Textures[8], uv);
        case 9: return texture(Textures[9], uv);
        case 10: return texture(Textures[10], uv);
        case 11: return texture(Textures[11], uv);
        case 12: return texture(Textures[12], uv);
        case 13: return texture(Textures[13], uv);
        case 14: return texture(Textures[14], uv);
        case 15: return texture(Textures[15], uv);
    }
    return vec4(1.0, 0.0, 1.0, 1.0);
}

void main(){
    if(TextureIndex == 1) {
        highp vec2 uv = (fract(TexCoords) + AtlasOffset) / AtlasSize;
        FragColor =  sampleTexture(TextureIndex, uv) * VertexColour;
    } else if(TextureIndex == 2) {
        float smoothing = 0.0035f;
        FragColor = VertexColour;
        FragColor.a *= smoothstep( 0.5 - smoothing, 0.5 + smoothing, sampleTexture(TextureIndex, TexCoords).r );;
    } else {
        FragColor = sampleTexture(TextureIndex, TexCoords) * VertexColour;
    }
})";
}