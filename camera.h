#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera{
private:

public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    
    float vel;
    float rot_speed;

    Camera(){
        
    }

    Camera(glm::vec3 &pos, glm::vec3 &tar, float speed, float rot){
        position = pos;
        front = tar;
        vel = speed;
        rot_speed = rot;
        up = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    glm::mat4 viewAtMat(){
        return glm::lookAt(position, position + front, up);
    }

    void update(glm::vec2&, glm::vec2&, float&);

};