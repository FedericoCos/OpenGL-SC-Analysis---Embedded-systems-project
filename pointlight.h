#pragma once
#include "light.h"


class PointLight : Light {
public:
    PointLight() : Light(){

    }

    ~PointLight() {

    }

    PointLight(glm::vec3& pos, glm::vec4& col, glm::vec3& vel) : Light(pos, col, vel){
        
    }

    void update(float& dtime);

    void setIndex(unsigned int i){
        index = i;
    }

    void linkShader(Shader& shader) override;

private:
    unsigned int index;

};