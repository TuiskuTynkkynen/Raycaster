#version 330 core

struct  Material{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

struct SpotLight {    
    vec3 position;
    vec3 direction;

    float cutOff;
    float outerCutOff;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffuseMap, vec3 specularMap);  
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 diffuseMap, vec3 specularMap);  
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 diffuseMap, vec3 specularMap);  

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TextureCoordinates;  

uniform Material material;
uniform DirLight dirLight;
#define NR_POINT_LIGHTS 4  
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight; 

out vec4 FragColor;

void main(){
    vec3 viewDir = normalize(-FragPos);
    
    vec3 diffuseMap = vec3(texture(material.diffuse, TextureCoordinates));
    vec3 specularMap = vec3(texture(material.specular, TextureCoordinates));

    vec3 result = CalcDirLight(dirLight, Normal, viewDir, diffuseMap, specularMap);
    
    for(int i = 0; i < NR_POINT_LIGHTS; i++){
        result += CalcPointLight(pointLights[i], Normal, viewDir, diffuseMap, specularMap);    
    }
    
    result += CalcSpotLight(spotLight, Normal, viewDir, diffuseMap, specularMap);    
    
	FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffuseMap, vec3 specularMap){
    vec3 lightDir = normalize(-light.direction);

    vec3 ambient  = light.ambient  * diffuseMap;

    float diff = max(dot(normal, lightDir), 0.0);    
    vec3 diffuse  = light.diffuse  * diff * diffuseMap;
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularMap;
    
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 diffuseMap, vec3 specularMap){
    vec3 lightDir = normalize(light.position - FragPos);
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    vec3 ambient  = light.ambient  * diffuseMap * attenuation;
    
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse  = light.diffuse  * diff * diffuseMap * attenuation;
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularMap * attenuation;
    
    return (ambient + diffuse + specular);
} 

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 diffuseMap, vec3 specularMap){
    vec3 lightDir = normalize(light.position - FragPos); 
    float theta = dot(lightDir, light.direction);

    float epsilon = light.cutOff - light.outerCutOff;
    float attenuation = 2 * clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);  

    vec3 ambient = light.ambient * diffuseMap * attenuation;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuseMap * attenuation;
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularMap * attenuation;

    return (ambient + diffuse + specular);
}