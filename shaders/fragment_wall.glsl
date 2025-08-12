#version 330 core
#define MAX_LIGHTS 100

out vec4 FragColor;

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
uniform Light pointLights[MAX_LIGHTS];
uniform int pointLights_num;
uniform vec3 viewPos;

struct SpotLight{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec4 color;
};
uniform SpotLight spotLights[1];


in vec3 Normal;
in vec3 FragPos;

in vec3 Color;

in vec4 FragPosLightSpace;

uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace){
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0)
        return 0.0;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float bias = 0.00005;
    float shadow = currentDepth > closestDepth + bias ? 1.0 : 0.0;

    return shadow;
}

void main(){
    vec3 ambient = vec3(0.0);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 ambientLightDirection = normalize(-directionalLight.direction);

    ambient += directionalLight.ambient.w * directionalLight.ambient.xyz * Color;
    /* diffuse += max(dot(norm, ambientLightDirection), 0.0) * directionalLight.diffuse.xyz * Color;
    specular += directionalLight.specular.w * pow(max(dot(viewDir, reflect(-ambientLightDirection, norm)), 0.0), 32) * directionalLight.specular.xyz * Color; */


    for (int i = 0; i < pointLights_num; i++) {
        // Distance from fragment to light
        float distance = length(pointLights[i].position - FragPos);

        // Attenuation
        float attenuation = 1.0 / (1.0 +
                                0.07 * distance +
                                0.017 * (distance * distance));

        // Ambient (affected by attenuation)
        vec3 ambientTerm = pointLights[i].color.w * pointLights[i].color.xyz * Color;

        // Diffuse
        vec3 lightDir = normalize(pointLights[i].position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuseTerm = diff * pointLights[i].color.xyz * Color;

        // Specular
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specularTerm =  spec * pointLights[i].color.w * pointLights[i].color.xyz * Color;

        // Apply attenuation
        ambient  += ambientTerm  * attenuation;
        diffuse  += diffuseTerm  * attenuation;
        specular += specularTerm * attenuation;
    }

    // Distance from fragment to light
    float distance = length(spotLights[0].position - FragPos);

    // Attenuation
    float attenuation = 1.0 / (1.0 +
                                0.022 * distance +
                                0.0019 * (distance * distance));

    vec3 lightDir = normalize(spotLights[0].position - FragPos);

    float theta = dot(lightDir, normalize(-spotLights[0].direction));
    float epsilon = spotLights[0].cutOff - spotLights[0].outerCutOff;
    float intensity = clamp((theta - spotLights[0].outerCutOff) / epsilon, 0.0, 1.0);
    
    // Ambient (affected by attenuation)
    vec3 ambientTerm = spotLights[0].color.w * spotLights[0].color.xyz * Color;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuseTerm = diff * spotLights[0].color.xyz * Color;

    // Specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specularTerm = spotLights[0].color.w * spec * spotLights[0].color.xyz * Color;

    // Apply intensity
    float shadow = ShadowCalculation(FragPosLightSpace);
    ambient  += ambientTerm  * intensity * attenuation;
    diffuse  += diffuseTerm  * intensity * attenuation;
    specular += specularTerm * intensity * attenuation;


    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular));
    FragColor = vec4(result, 1.0);
}