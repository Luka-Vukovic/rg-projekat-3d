#version 330 core
out vec4 FragColor;

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;

uniform sampler2D uDiffMap1;
uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform vec3 uLightColor;

// Uniforme za lighting system iz tvog projekta
struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    float constant;
    float linear;
    float quadratic;
};

#define MAX_LIGHTS 10
uniform Light lights[MAX_LIGHTS];
uniform int numLights;

vec3 calculateLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 objectColor) {
    // Ambient
    vec3 ambient = light.ambient * objectColor;
    
    // Diffuse
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * objectColor;
    
    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = light.specular * spec * 0.5; // Manje specular za ljude
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return ambient + diffuse + specular;
}

void main()
{
    vec3 objectColor = texture(uDiffMap1, TexCoords).rgb;
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(uViewPos - FragPos);
    
    vec3 result = vec3(0.0);
    for(int i = 0; i < numLights; i++) {
        result += calculateLight(lights[i], norm, FragPos, viewDir, objectColor);
    }
    
    FragColor = vec4(result, 1.0);
}