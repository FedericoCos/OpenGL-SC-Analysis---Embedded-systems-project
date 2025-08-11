#pragma once
#include "light.h"

class AmbientLight : Light {
public:
    AmbientLight() : Light() {
        direction = glm::vec3(1.0f, -1.f, 0.f);
        diffuse_fact = 0.5f;
        specular_fact = 2.f;
    }

    AmbientLight(glm::vec3& pos, glm::vec4& col, glm::vec3& vel, glm::vec3& dir, float& diff_fact, float& spec_fact) 
        : Light(pos, col, vel) {
            direction = dir;
            diffuse_fact = diff_fact;
            specular_fact = spec_fact;
    }

    ~AmbientLight(){

    }

    void update(float& dtime) override;

    void linkShader(Shader& shader) override;

    glm::vec3& getDirection(){
        return direction;
    }

    glm::vec4& getAmbient(){
        return color;
    }

    glm::vec4 getDiffuse(){
        return glm::vec4(glm::vec3(color), diffuse_fact);
    }

    glm::vec4 getSpecular(){
        return glm::vec4(glm::vec3(color), specular_fact);
    }


private:
    glm::vec3 direction;
    float diffuse_fact;
    float specular_fact;

};