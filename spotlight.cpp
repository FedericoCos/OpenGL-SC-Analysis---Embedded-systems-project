#include "spotlight.h"

void SpotLight::update(float& dtime){
    position += velocity * dtime;

    mat = glm::lookAt(position, position + direction, up);
}

void SpotLight::align_light(glm::vec3 pos, glm::vec3 dir, glm::vec3 up){
    position = pos;
    direction = dir;
    this -> up = up;
}

void SpotLight::linkShader(Shader& shader){
    shader.setVec3("spotLights[" + std::to_string(index) + "].position", position);
    shader.setVec3("spotLights[" + std::to_string(index) + "].direction", direction);
    shader.setFloat("spotLights[" + std::to_string(index) + "].cutOff", glm::cos(glm::radians(12.5f)));
    shader.setFloat("spotLights[" + std::to_string(index) + "].outerCutOff", glm::cos(glm::radians(22.5f)));


    shader.setVec4("spotLights[" + std::to_string(index) + "].color", color);
}

