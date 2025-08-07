#version 330 core
#define MAX_LIGHTS 100

out vec4 FragColor;

struct DirectionalLight{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirectionalLight directionalLight;


struct Light{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform Light lights[MAX_LIGHTS];
uniform int numLights;
uniform vec3 viewPos;

struct SpotLight{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform SpotLight spotLight;


in vec3 Normal;
in vec3 FragPos;

in vec3 Color;

void main(){
    vec3 ambient = vec3(0.0);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    float ambientStrength = 0.2;
    float specularStrength = 0.5;

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 ambientLightDirection = normalize(-directionalLight.direction);

    /* ambient += ambientStrength * directionalLight.ambient * Color;
    diffuse += max(dot(norm, ambientLightDirection), 0.0) * directionalLight.diffuse * Color;
    specular += specularStrength * pow(max(dot(viewDir, reflect(-ambientLightDirection, norm)), 0.0), 32) * directionalLight.specular * Color;
 */

    for (int i = 0; i < numLights; i++) {
        // Distance from fragment to light
        float distance = length(lights[i].position - FragPos);

        // Attenuation
        float attenuation = 1.0 / (1.0 +
                                0.07 * distance +
                                0.017 * (distance * distance));

        // Ambient (affected by attenuation)
        vec3 ambientTerm = ambientStrength * lights[i].ambient * Color;

        // Diffuse
        vec3 lightDir = normalize(lights[i].position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuseTerm = diff * lights[i].diffuse * Color;

        // Specular
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specularTerm = specularStrength * spec * lights[i].specular * Color;

        // Apply attenuation
        ambient  += ambientTerm  * attenuation;
        diffuse  += diffuseTerm  * attenuation;
        specular += specularTerm * attenuation;
    }

    // Distance from fragment to light
    float distance = length(spotLight.position - FragPos);

    // Attenuation
    float attenuation = 1.0 / (1.0 +
                                0.022 * distance +
                                0.0019 * (distance * distance));

    vec3 lightDir = normalize(spotLight.position - FragPos);

    float theta = dot(lightDir, normalize(-spotLight.direction));
    float epsilon = spotLight.cutOff - spotLight.outerCutOff;
    float intensity = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0);
    
    // Ambient (affected by attenuation)
    vec3 ambientTerm = ambientStrength * spotLight.ambient * Color;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuseTerm = diff * spotLight.diffuse * Color;

    // Specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specularTerm = specularStrength * spec * spotLight.specular * Color;

    // Apply intensity
    ambient  += ambientTerm  * intensity*attenuation;
    diffuse  += diffuseTerm  * intensity * attenuation;
    specular += specularTerm * intensity * attenuation;

    vec3 result = (ambient + diffuse + specular);
    FragColor = vec4(result, 1.0);
}