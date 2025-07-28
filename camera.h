#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera{
private:

public:
    glm::vec3 position;
    glm::vec3 front;
    
    float vel;

    Camera(glm::vec3 &pos, glm::vec3 &tar, float speed){
        position = pos;
        front = tar;
        vel = speed;
    }

    glm::mat4 viewAtMat(){
        return glm::lookAt(position, position + front, glm::vec3(.0f, 1.0f, 0.f));
    }

};