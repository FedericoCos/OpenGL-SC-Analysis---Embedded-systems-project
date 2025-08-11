#include "wall.h"


void Wall::init(glm::vec3& pos, glm::vec3& scale, glm::vec4& rot){
    float wall_vertices[36] = {
        // positions           // normals            // colors
        1.0f,  1.0f, 0.0f,    0.0f, 0.0f,1.0f,    0.5f, 0.5f, 0.5f,
        -1.0f,  1.0f, 0.0f,    0.0f, 0.0f,1.0f,    0.5f, 0.5f, 0.5f,
        -1.0f, -1.0f, 0.0f,    0.0f, 0.0f,1.0f,    0.5f, 0.5f, 0.5f,
        1.0f, -1.0f, 0.0f,    0.0f, 0.0f,1.0f,    0.5f, 0.5f, 0.5f
    };

    unsigned int wall_indices[6] = {
        0, 1, 2,
        0, 2, 3
    };
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(wall_vertices), wall_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(wall_indices), wall_indices, GL_STATIC_DRAW);

    position = pos;
    this -> scale = scale;
    rotation = rot;
    update_model_mat();
}


void Wall::draw(){
    if(!is_static){
        update_model_mat();
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    shader.setMatrix("model", model_matrix);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}


void Wall::update(glm::vec3& pos, glm::vec3& scale, glm::vec4& rot){
    if(is_static){
        std::cerr << "Tried to update a Static object. Object of wall type" << std::endl;
        return;
    }

    position = pos;
    this -> scale = scale;
    rotation = rot;
}

void Wall::draw(Shader& s){
    if(!is_static){
        update_model_mat();
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    s.setMatrix("model", model_matrix);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Wall::set_lights(AmbientLight& ambient_light, std::vector<PointLight> &pointLights, std::vector<SpotLight> &spotLights){
    ambient_light.linkShader(shader);

    shader.setInt("pointLights_num", pointLights.size());
    for(size_t i = 0; i < pointLights.size(); i++){
        pointLights[i].setIndex(i);
        pointLights[i].linkShader(shader);
    }

    shader.setInt("spotLights_num", spotLights.size());
    for(size_t i = 0; i < spotLights.size(); i++){
        spotLights[i].setIndex(i);
        spotLights[i].linkShader(shader);
    }
}

