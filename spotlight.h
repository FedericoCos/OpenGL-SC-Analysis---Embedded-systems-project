#pragma once
#include "light.h"

class SpotLight : Light {
public:
    SpotLight() : Light() {
        inn_cut_off = 17.5f;
        out_cut_off = 22.5f;
        direction = glm::vec3(0.0f, 0.f, -1.f);
    }

    ~SpotLight() {

    }

    SpotLight(glm::vec3& pos, glm::vec4& col, glm::vec3& vel, glm::vec3& dir, float i_c_o, float o_c_o) : Light(pos, col, vel) {
        inn_cut_off = i_c_o;
        out_cut_off = o_c_o;

        direction = direction;
    }

    glm::vec3& getDirection(){
        return direction;
    }

    glm::mat4 getLookAt(){
        return glm::lookAt(position, position + direction, up);
    }

    void update(float& dtime);
    void align_light(glm::vec3 pos, glm::vec3 dir, glm::vec3 up);

    void linkShader(Shader& shader) override;

    void setIndex(int i){
        index = i;
    }

private:
    float inn_cut_off;
    float out_cut_off;

    glm::vec3 direction;
    glm::vec3 up;
    glm::mat4 mat;

    int index;

};