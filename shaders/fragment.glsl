#version 330 core
#define MAX_LIGHTS 100

out vec4 FragColor;

uniform vec3 objectColor;
struct Material{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};
uniform Material material;
in vec2 TexCoords;

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

in vec3 Normal;
in vec3 FragPos;
  
void main()
{
    vec3 ambient = vec3(0.0);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    float ambientStrength = 0.2;
    float specularStrength = 0.5;

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 diffuseVec = vec3(texture(material.diffuse, TexCoords));
    vec3 specularVec = vec3(texture(material.specular, TexCoords));


    // -------- DIRECTIONAL LIGHT
    vec3 ambientLightDirection = normalize(-directionalLight.direction);

    ambient += ambientStrength * directionalLight.ambient * diffuseVec;
    diffuse += max(dot(norm, ambientLightDirection), 0.0) * directionalLight.diffuse * diffuseVec;
    specular += specularStrength * pow(max(dot(viewDir, reflect(-ambientLightDirection, norm)), 0.0), 32) * directionalLight.specular * specularVec;


    for (int i = 0; i < numLights; i++) {
        // Distance from fragment to light
        float distance = length(lights[i].position - FragPos);
        
        // Attenuation
        float attenuation = 1.0 / (1.0 +
                                0.07 * distance +
                                0.017 * (distance * distance));

        // Ambient (affected by attenuation)
        vec3 ambientTerm = ambientStrength * lights[i].ambient * diffuseVec;

        // Diffuse
        vec3 lightDir = normalize(lights[i].position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuseTerm = diff * lights[i].diffuse * diffuseVec;

        // Specular
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specularTerm = specularStrength * spec * lights[i].specular * specularVec;

        // Apply attenuation
        ambient  += ambientTerm  * attenuation;
        diffuse  += diffuseTerm  * attenuation;
        specular += specularTerm * attenuation;
    }

    vec3 result = (ambient + diffuse + specular);
    FragColor = vec4(result, 1.0);
}