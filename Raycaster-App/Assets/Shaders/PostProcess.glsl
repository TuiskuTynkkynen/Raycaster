#shader vertex

#version 460 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTexturePosition;

out vec2 TexturePosition;

void main()
{
     gl_Position = vec4(aPosition, 1.0f, 1.0f);
     TexturePosition= aTexturePosition;
}


#shader fragment

#version 460 core

in vec2 TexturePosition;

layout(binding = 0) uniform sampler2D Texture;

out vec4 FragColor;

// Attribution: https://github.com/dmnsgn/glsl-tone-map/blob/main/lottes.glsl.js
vec3 lottes(vec3 x) {
    const vec3 a = vec3(1.4); // Smaller value seemed nicer to me
    const vec3 d = vec3(0.977);

    const vec3 hdrMax = vec3(8.0);
    const vec3 midIn = vec3(0.18);
    const vec3 midOut = vec3(0.267);

    const vec3 b =
        (-pow(midIn, a) + pow(hdrMax, a) * midOut) /
        ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);
    const vec3 c =
        (pow(hdrMax, a * d) * pow(midIn, a) - pow(hdrMax, a) * pow(midIn, a * d) * midOut) /
        ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);

    return pow(x, a) / (pow(x, a * d) * b + c);
}

void main() {
    const float gamma = 2.2;
  
    vec3 hdrColor = texture(Texture, TexturePosition).rgb;
    hdrColor = pow(hdrColor, vec3(gamma)); // undo gamma correction
    
    vec3 mapped = lottes(hdrColor);
    mapped  = pow(mapped, vec3(1.0 / gamma));  // redo gamma correction
    
    FragColor = vec4(mapped, 1.0);
}
