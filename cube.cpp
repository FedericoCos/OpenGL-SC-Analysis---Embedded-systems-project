#include "cube.h"

void Cube::init(glm::vec3& pos, glm::vec3& scale, glm::vec4& rot){
    float cube_vertices[144] = {
        // Front face (z = 0.5)
        0.5f, 0.5f, 0.5f,  0.f,  0.f,  1.f,   // 0 top right
        0.5f,-0.5f, 0.5f,  0.f,  0.f,  1.f,    // 1 bottom right
        -0.5f,-0.5f, 0.5f,  0.f,  0.f,  1.f,    // 2 bottom left
        -0.5f, 0.5f, 0.5f,  0.f,  0.f,  1.f,    // 3 top left
        // Back face (z = -0.5)
        0.5f, 0.5f,-0.5f,  0.f,  0.f, -1.f,    // 4 top right
        0.5f,-0.5f,-0.5f,  0.f,  0.f, -1.f,    // 5 bottom right
        -0.5f,-0.5f,-0.5f,  0.f,  0.f, -1.f,    // 6 bottom left
        -0.5f, 0.5f,-0.5f,  0.f,  0.f, -1.f,    // 7 top left
        // Left face (x = -0.5)
        -0.5f, 0.5f,-0.5f, -1.f,  0.f,  0.f,    // 8 top right
        -0.5f,-0.5f,-0.5f, -1.f,  0.f,  0.f,    // 9 bottom right
        -0.5f,-0.5f, 0.5f, -1.f,  0.f,  0.f,    //10 bottom left
        -0.5f, 0.5f, 0.5f, -1.f,  0.f,  0.f,    //11 top left
        // Right face (x = 0.5)
        0.5f, 0.5f, 0.5f,  1.f,  0.f,  0.f,    //12 top left
        0.5f,-0.5f, 0.5f,  1.f,  0.f,  0.f,    //13 bottom left
        0.5f,-0.5f,-0.5f,  1.f,  0.f,  0.f,    //14 bottom right
        0.5f, 0.5f,-0.5f,  1.f,  0.f,  0.f,    //15 top right
        // Top face (y = 0.5)
        0.5f, 0.5f,-0.5f,  0.f,  1.f,  0.f,    //16 top right
        0.5f, 0.5f, 0.5f,  0.f,  1.f,  0.f,    //17 bottom right
        -0.5f, 0.5f, 0.5f,  0.f,  1.f,  0.f,    //18 bottom left
        -0.5f, 0.5f,-0.5f,  0.f,  1.f,  0.f,    //19 top left
        // Bottom face (y = -0.5)
        0.5f,-0.5f, 0.5f,  0.f, -1.f,  0.f,    //20 bottom right
        0.5f,-0.5f,-0.5f,  0.f, -1.f,  0.f,    //21 top right
        -0.5f,-0.5f,-0.5f,  0.f, -1.f,  0.f,    //22 top left
        -0.5f,-0.5f, 0.5f,  0.f, -1.f,  0.f,    //23 bottom left
    };

    unsigned int cube_indices[36] = {
        // Front face
        0, 1, 2,
        2, 3, 0,

        // Back face
        4, 5, 6,
        6, 7, 4,

        // Left face
        8, 9,10,
        10,11,8,

        // Right face
        12,13,14,
        14,15,12,

        // Top face
        16,17,18,
        18,19,16,

        // Bottom face
        20,21,22,
        22,23,20
    };

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);


    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    posLoc = glGetAttribLocation(shader.ID, "aPos");
    normLoc = glGetAttribLocation(shader.ID, "aNormal");

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    position = pos;
    this -> scale = scale;
    rotation = rot;
    update_model_mat();
}

void Cube::draw(){
    if(!is_static){
        update_model_mat();
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // vertex position
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void *)0);
    glEnableVertexAttribArray(posLoc);

    // vertex normal
    glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(normLoc);

    shader.setMatrix("model", model_matrix);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

void Cube::update(glm::vec3& pos, glm::vec3& scale, glm::vec4& rot){
    if(is_static){
        std::cerr << "Tried to update a Static object. Object of wall type" << std::endl;
        return;
    }

    position = pos;
    this -> scale = scale;
    rotation = rot;
}

void Cube::draw(Shader& s){
    if(!is_static){
        update_model_mat();
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // vertex position
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void *)0);
    glEnableVertexAttribArray(posLoc);

    // vertex normal
    glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(normLoc);

    s.setMatrix("model", model_matrix);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

void Cube::set_lights(AmbientLight& ambient_light, std::vector<PointLight> &pointLights, std::vector<SpotLight> &spotLights){
    ambient_light.linkShader(shader);

    shader.setInt("pointLights_num", pointLights.size());
    for(size_t i = 0; i < pointLights.size(); i++){
        pointLights[i].setIndex(i);
        pointLights[i].linkShader(shader);
    }

    shader.setInt("spotLights_num", spotLights.size());
    for(size_t i = 0; i < spotLights.size(); i++){
        spotLights[i].linkShader(shader);
    }
}
