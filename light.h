#pragma once
#include "helpers.h"
#include "shader.h"



class Light{
public:
    Light(glm::vec3& pos, glm::vec4& col, glm::vec3& vel){
        position = pos;
        color = col;
        velocity = vel;
    }

    Light(){
        position = glm::vec3(0.f);
        color = glm::vec4(1.f);
        velocity = glm::vec3(0.f);
    }

    ~Light(){

    };

    glm::vec3& getPosition(){
        return position;
    }

    glm::vec4& getColor(){
        return color;
    }

    glm::vec3& getVelocity(){
        return velocity;
    }

    virtual void update(float& dtime) = 0;

    virtual void linkShader(Shader& shader) = 0;


protected:
    glm::vec3 position;
    glm::vec4 color; // w is for the intensity
    glm::vec3 velocity;
};
