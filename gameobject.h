#pragma once

#include "helpers.h"
#include "shader.h"

#include "ambientlight.h"
#include "pointlight.h"
#include "spotlight.h"


class GameObject {
public:
    GameObject() {
        
    }

    ~GameObject() {

    }

    GameObject(const char * vertex_shader, const char * fragment_shader, bool stat){
        shader = Shader(vertex_shader, fragment_shader);
        is_static = stat;
    }

    Shader& getShader(){
        return shader;
    }

    virtual void init(glm::vec3& pos, glm::vec3& scale, glm::vec4& rot) = 0;

    virtual void draw() = 0;

    virtual void draw(Shader& s) = 0; // In case of needing a different shader defined outside

    virtual void update(glm::vec3& pos, glm::vec3& scale, glm::vec4& rotation) = 0;

    virtual void set_lights(AmbientLight& ambient_light, std::vector<PointLight> &pointLights, std::vector<SpotLight> &potLights) = 0;

protected:
    glm::mat4 model_matrix;
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec4 rotation;

    bool is_static; // whether the model matrix changes between frames or not


    unsigned int VBO;
    unsigned int EBO;
    Shader shader;


    void update_model_mat(){
        model_matrix = glm::mat4(1.f);
        model_matrix = glm::translate(model_matrix, position);
        model_matrix = glm::scale(model_matrix, scale);
        model_matrix = glm::rotate(model_matrix, glm::radians(rotation.w), glm::vec3(rotation));
    }
};

