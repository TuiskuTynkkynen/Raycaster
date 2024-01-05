#version 330 core

in vec3 vertexColour;
in vec2 textureCoordinates;

uniform sampler2D currentTexture0;
uniform sampler2D currentTexture1;

out vec4 FragColor;

void main(){
    vec4 textureMix = mix(texture(currentTexture0, textureCoordinates), texture(currentTexture1, textureCoordinates), 0.5f);
    FragColor = mix(textureMix, vec4(vertexColour, 1.0f), 0.5f);
}