#version 100
// GLSL for OpenGL ES 2.0

// GLES fragment shaders require a default precision
precision mediump float;

#define MAX_LIGHTS 100

// Varyings (inputs from vertex shader)
varying vec2 TexCoords;
varying vec3 Normal;
varying vec3 FragPos;
varying vec4 FragPosLightSpace;

// Material and Light structs are compatible
struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
};

struct Light {
    vec3 position;
    vec4 color;
};

struct DirectionalLight {
    vec3 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    vec4 color;
};

// Uniforms
uniform Material material;
uniform Light pointLights[MAX_LIGHTS];
uniform int pointLights_num;
uniform DirectionalLight directionalLight;
uniform SpotLight spotLights[2];
uniform vec3 viewPos;
uniform sampler2D shadowMap;

void main()
{
    vec3 ambient = vec3(0.0);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Use texture2D for GLES 2.0
    vec3 diffuseVec  = vec3(texture2D(material.texture_diffuse1, TexCoords));
    vec3 specularVec = vec3(texture2D(material.texture_specular1, TexCoords));

    // ----- Directional Light -----
    ambient += directionalLight.ambient.w * directionalLight.ambient.xyz * diffuseVec;

    // ----- Point Lights -----
    for (int i = 0; i < pointLights_num; i++) {
        float distance = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (1.0 + 0.07 * distance + 0.017 * (distance * distance));

        vec3 lightDir = normalize(pointLights[i].position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        diffuse += (diff * pointLights[i].color.xyz * diffuseVec) * attenuation;

        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        specular += (spec * pointLights[i].color.w * pointLights[i].color.xyz * specularVec) * attenuation;
    }

    // ----- Spot Light -----
    float distance = length(spotLights[0].position - FragPos);
    float attenuation = 1.0 / (1.0 + 0.022 * distance + 0.0019 * (distance * distance));

    vec3 lightDir = normalize(spotLights[0].position - FragPos);
    float theta = dot(lightDir, normalize(-spotLights[0].direction));
    float epsilon = spotLights[0].cutOff - spotLights[0].outerCutOff;
    float intensity = clamp((theta - spotLights[0].outerCutOff) / epsilon, 0.0, 1.0);
    
    float diff = max(dot(norm, lightDir), 0.0);
    diffuse += (diff * spotLights[0].color.xyz * diffuseVec) * attenuation * intensity;

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    specular += (spotLights[0].color.w * spec * spotLights[0].color.xyz * specularVec) * attenuation * intensity;

    // Final Calculation
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    // Use texture2D for GLES 2.0
    float closestDepth = texture2D(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    
    // Bias might need to be larger for GLES depth formats
    float bias = 0.0005;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;


    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);
    
    // Output to the built-in gl_FragColor
    gl_FragColor = vec4(result, 1.0);
}
