#shader vertex

#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColour;
layout (location = 2) in vec2 aTexPos;
layout (location = 3) in vec2 aAtlasOffset;
layout (location = 4) in float aTexIndex;

uniform mat4 ViewProjection;

out vec4 VertexColour;

out vec2 TexCoords;
out vec2 AtlasOffset;
flat out int TextureIndex;

void main()
{
    gl_Position = ViewProjection * vec4(aPos, 1.0f);

    VertexColour = aColour;

    TexCoords = aTexPos;
    AtlasOffset = aAtlasOffset;
    AtlasOffset += 1e-5;
    TextureIndex = int(aTexIndex);
}


#shader fragment

#version 460 core

in vec4 VertexColour;

in vec2 TexCoords;
in vec2 AtlasOffset;
flat in int TextureIndex;

uniform sampler2D Textures[16];
uniform vec2 AtlasSize;

out vec4 FragColor;

void main(){
    if(TextureIndex == 1) {
        vec2 uv = (fract(TexCoords) + AtlasOffset) / AtlasSize;
        FragColor =  texture(Textures[TextureIndex], uv) * VertexColour;
        return;
    } else if(TextureIndex == 2) {
        float smoothing = 0.0035f;
        float alpha = smoothstep( 0.5 - smoothing, 0.5 + smoothing, texture(Textures[TextureIndex], TexCoords).r );
        FragColor = VertexColour;
        FragColor.a *= alpha;
        return;
    }

    FragColor = texture(Textures[TextureIndex], TexCoords) * VertexColour;
}
