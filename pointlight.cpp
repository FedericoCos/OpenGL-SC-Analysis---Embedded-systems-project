#include "pointlight.h"


void PointLight::update(float& dtime){
    position += velocity * dtime;
}

void PointLight::linkShader(Shader& shader){
    shader.setVec3("pointLights[" + std::to_string(index) + "].position", position);
    shader.setVec4("pointLights[" + std::to_string(index) + "].color", color);
}