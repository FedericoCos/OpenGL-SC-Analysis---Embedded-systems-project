#pragma once
#include "gameobject.h"

class Wall : GameObject {
public:

    Wall() : GameObject() {

    }

    ~Wall(){

    }

    Wall(const char * vertex_shader, const char * fragment_shader, bool stat) : GameObject(vertex_shader, fragment_shader, stat) {

    }

    void init(glm::vec3& pos, glm::vec3& scale, glm::vec4& rot) override;
    void draw() override;
    void draw(Shader& s) override;

    void update(glm::vec3& pos, glm::vec3& scale, glm::vec4& rot) override;

    void set_lights(AmbientLight& ambient_light, std::vector<PointLight> &pointLights, std::vector<SpotLight> &potLights) override;

    Shader& getShader(){
        return GameObject::getShader();
    }

private:

};