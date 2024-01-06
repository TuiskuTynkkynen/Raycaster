#version 330 core

uniform vec3 objectColor;
uniform vec3 lightColor;

out vec4 FragColor;

void main(){
	FragColor = vec4(0.8, 0.8, 1.0, 1.0);
}