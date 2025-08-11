#version 330 core
#define MAX_LIGHTS 100
out vec4 FragColor;

in vec2 TexCoords;

struct Material{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
};
uniform Material material;

// Match first shader's struct
struct Light{
    vec3 position;
    vec4 color; // w = strength, xyz = RGB
};
uniform Light pointLights[10];
uniform int pointLights_num;

struct DirectionalLight{
    vec3 direction;
    vec4 ambient;  // w = strength, xyz = RGB
    vec4 diffuse;
    vec4 specular;
};
uniform DirectionalLight directionalLight;

struct SpotLight{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    vec4 color; // w = strength, xyz = RGB
};
uniform SpotLight spotLights[2];

uniform vec3 viewPos;

in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace){
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0)
        return 0.0;
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    float bias = 0.00001;
    float shadow = currentDepth > closestDepth + bias ? 1.0 : 0.0;
    return shadow;
}

void main()
{
    vec3 ambient = vec3(0.0);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 diffuseVec  = vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specularVec = vec3(texture(material.texture_specular1, TexCoords));

    // ----- Directional Light -----
    vec3 ambientLightDirection = normalize(-directionalLight.direction);

    ambient  += directionalLight.ambient.w * directionalLight.ambient.xyz * diffuseVec;
    /* diffuse  += max(dot(norm, ambientLightDirection), 0.0) * directionalLight.diffuse.xyz * diffuseVec;
    specular += directionalLight.specular.w * pow(max(dot(viewDir, reflect(-ambientLightDirection, norm)), 0.0), material.shininess) 
                * directionalLight.specular.xyz * specularVec; */

    // ----- Point Lights -----
    for (int i = 0; i < pointLights_num; i++) {
        float distance = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (1.0 +
                                   0.07 * distance +
                                   0.017 * (distance * distance));

        vec3 ambientTerm = pointLights[i].color.w * pointLights[i].color.xyz * diffuseVec;

        vec3 lightDir = normalize(pointLights[i].position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuseTerm = diff * pointLights[i].color.xyz * diffuseVec;

        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specularTerm = spec * pointLights[i].color.w * pointLights[i].color.xyz * specularVec;

        ambient  += ambientTerm  * attenuation;
        diffuse  += diffuseTerm  * attenuation;
        specular += specularTerm * attenuation;
    }

    // ----- Spot Light -----
    float distance = length(spotLights[0].position - FragPos);
    float attenuation = 1.0 / (1.0 +
                               0.022 * distance +
                               0.0019 * (distance * distance));

    vec3 lightDir = normalize(spotLights[0].position - FragPos);
    float theta = dot(lightDir, normalize(-spotLights[0].direction));
    float epsilon = spotLights[0].cutOff - spotLights[0].outerCutOff;
    float intensity = clamp((theta - spotLights[0].outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambientTerm = spotLights[0].color.w * spotLights[0].color.xyz * diffuseVec;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuseTerm = diff * spotLights[0].color.xyz * diffuseVec;

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specularTerm = spotLights[0].color.w * spec * spotLights[0].color.xyz * specularVec;

    float shadow = ShadowCalculation(FragPosLightSpace);
    ambient  += ambientTerm  * intensity * attenuation;
    diffuse  += diffuseTerm  * intensity * attenuation;
    specular += specularTerm * intensity * attenuation;

    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);
    FragColor = vec4(result, 1.0);
}
