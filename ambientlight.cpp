#include "ambientlight.h"

void AmbientLight::update(float& dtime){
    position += velocity * dtime;
}

void AmbientLight::linkShader(Shader& shader){
    glm::vec4 diffuse = getDiffuse();
    glm::vec4 specular = getSpecular();


    shader.setVec3("directionalLight.direction", direction);
    shader.setVec4("directionalLight.ambient", getAmbient());
    shader.setVec4("directionalLight.diffuse", diffuse);
    shader.setVec4("directionalLight.specular", specular);
}
