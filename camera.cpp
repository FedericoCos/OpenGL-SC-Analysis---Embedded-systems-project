#include "camera.h"
#include <iostream>

void Camera::update(glm::vec2& dis, glm::vec2& look, float& delta_time){
    glm::vec3 delta_pos(0.f);
    glm::mat4 rot(1.f);

    if(look.y > 0.0f){
        rot = glm::rotate(rot, -rot_speed * delta_time, up);
    }
    else if(look.y < 0.0f){
        rot = glm::rotate(rot, rot_speed * delta_time, up);
    }

    if(look.x > 0.0f){
        rot = glm::rotate(rot, rot_speed * delta_time, glm::cross(front, up));
    }
    if(look.x < 0.0f){
        rot = glm::rotate(rot, -rot_speed * delta_time, glm::cross(front, up));
    }


    front = rot * glm::vec4(front, 1.0f);
    up = rot * glm::vec4(up, 1.0f);


    if(dis.x > 0.0f){
        delta_pos += glm::normalize(front);
    }
    else if(dis.x < 0.0f){
        delta_pos -= glm::normalize(front);
    }

    if(dis.y > 0.0f){
        delta_pos += glm::normalize(glm::cross(front, up));
    }
    else if(dis.y < 0.0f){
        delta_pos -= glm::normalize(glm::cross(front, up));
    }



    
    if(glm::length(delta_pos) > 0)
        position += vel * delta_time * glm::normalize(delta_pos);
}