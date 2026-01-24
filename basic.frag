#version 330 core

in vec4 channelCol;
in vec2 channelTex;
in vec3 fragPos;
in vec3 fragNormal;

out vec4 outCol;

uniform sampler2D uTex;
uniform bool useTex;
uniform bool transparent;

// Svetla
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
uniform vec3 viewPos;

vec3 calculateLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 objectColor) {
    // Ambient
    vec3 ambient = light.ambient * objectColor;
    
    // Diffuse
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * objectColor;
    
    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = light.specular * spec;
    
    // Atenuacija
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return ambient + diffuse + specular;
}

void main()
{
    vec4 baseColor;
    
    if (!useTex) {
        baseColor = channelCol;
    }
    else {
        baseColor = texture(uTex, channelTex);
        if (!transparent && baseColor.a < 1) {
            baseColor = vec4(1.0, 1.0, 1.0, 1.0);
        }
    }
    
    // Normalizuj normalu
    vec3 norm = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPos);
    
    // Kalkuliši osvjetljenje od svih svetala
    vec3 result = vec3(0.0);
    for(int i = 0; i < numLights; i++) {
        result += calculateLight(lights[i], norm, fragPos, viewDir, baseColor.rgb);
    }
    
    outCol = vec4(result, baseColor.a);
}