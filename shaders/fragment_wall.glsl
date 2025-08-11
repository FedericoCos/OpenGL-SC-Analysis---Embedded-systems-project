#version 100
// Note: GLSL ES 1.00 does not use version 'core'

// Fragment shaders in GLES require a default precision
precision mediump float;

#define MAX_LIGHTS 100

// Structs are supported
struct DirectionalLight{
    vec3 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};
uniform DirectionalLight directionalLight;

struct Light{
    vec3 position;
    vec4 color;
};
uniform Light pointLights[10];
uniform int pointLights_num;

struct SpotLight{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    vec4 color;
};
uniform SpotLight spotLights[2];

uniform vec3 viewPos;
uniform sampler2D shadowMap;

// 'in' becomes 'varying'
varying vec3 Normal;
varying vec3 FragPos;
varying vec3 Color;
varying vec4 FragPosLightSpace;


float ShadowCalculation(vec4 fragPosLightSpace){
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0)
        return 0.0;
    
    // 'texture' becomes 'texture2D' in GLSL ES 1.00
    float closestDepth = texture2D(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    
    // Bias might need tweaking on different hardware
    float bias = 0.0005;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

void main(){
    // The main logic is mostly fine, but the output must be to gl_FragColor
    vec3 ambient = vec3(0.0);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Ambient Light
    ambient += directionalLight.ambient.w * directionalLight.ambient.xyz * Color;

    // Point Lights
    for (int i = 0; i < pointLights_num; i++) {
        vec3 lightDir = normalize(pointLights[i].position - FragPos);
        float distance = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (1.0 + 0.07 * distance + 0.017 * (distance * distance));
        
        // Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        diffuse += (diff * pointLights[i].color.xyz * Color) * attenuation;
        
        // Specular
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        specular += (spec * pointLights[i].color.w * pointLights[i].color.xyz) * attenuation;
    }

    // Spot Light
    vec3 spotLightDir = normalize(spotLights[0].position - FragPos);
    float spotDistance = length(spotLights[0].position - FragPos);
    float spotAttenuation = 1.0 / (1.0 + 0.022 * spotDistance + 0.0019 * (spotDistance * spotDistance));
    
    float theta = dot(spotLightDir, normalize(-spotLights[0].direction));
    float epsilon = spotLights[0].cutOff - spotLights[0].outerCutOff;
    float intensity = clamp((theta - spotLights[0].outerCutOff) / epsilon, 0.0, 1.0);
    
    // Diffuse
    float spotDiff = max(dot(norm, spotLightDir), 0.0);
    diffuse += (spotDiff * spotLights[0].color.xyz * Color) * spotAttenuation * intensity;
    
    // Specular
    vec3 spotReflectDir = reflect(-spotLightDir, norm);
    float spotSpec = pow(max(dot(viewDir, spotReflectDir), 0.0), 32.0);
    specular += (spotLights[0].color.w * spotSpec * spotLights[0].color.xyz) * spotAttenuation * intensity;

    // Final Calculation
    float shadow = ShadowCalculation(FragPosLightSpace);
    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);

    // Final output must be to the built-in gl_FragColor
    gl_FragColor = vec4(result, 1.0);
}