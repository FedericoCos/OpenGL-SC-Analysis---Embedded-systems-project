#include "wall.h"
#include <iostream> // For std::cerr

// No changes to the header needed, only the implementation

void Wall::init(glm::vec3& pos, glm::vec3& scale, glm::vec4& rot){
    // Vertex data is fine
    float wall_vertices[] = {
        // positions         // normals         // colors
         1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.5f, 0.5f, 0.5f,
        -1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.5f, 0.5f, 0.5f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.5f, 0.5f, 0.5f,
         1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.5f, 0.5f, 0.5f
    };

    unsigned short wall_indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(wall_vertices), wall_vertices, GL_STATIC_DRAW);

    posLoc = glGetAttribLocation(shader.ID, "aPos");
    normLoc = glGetAttribLocation(shader.ID, "aNormal");
    colorLoc = glGetAttribLocation(shader.ID, "aColor");



    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(wall_indices), wall_indices, GL_STATIC_DRAW);

    // Unbind buffers for safety
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    position = pos;
    this->scale = scale;
    rotation = rot;
    update_model_mat();
}


void Wall::draw(){
    if(!is_static){
        update_model_mat();
    }

    shader.use(); // Use the shader associated with this wall
    shader.setMatrix("model", model_matrix);

    // --- FIX 2: Bind buffers and set vertex attributes before drawing ---
    // This ensures the correct vertex layout is active for this object.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // vertex position
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void *)0);
    glEnableVertexAttribArray(posLoc);

    // vertex color
    glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(6*sizeof(float)));
    glEnableVertexAttribArray(colorLoc);

    // vertex normal
    glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(normLoc);

    // --- FIX 3: Use GL_UNSIGNED_SHORT for the draw call ---
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

// Overloaded draw function for shadow pass
void Wall::draw(Shader& s){
    if(!is_static){
        update_model_mat();
    }

    s.use(); // Use the provided shader (e.g., the depth shader)
    s.setMatrix("model", model_matrix);

    // Also set up the vertex attributes for the shadow pass
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // vertex position
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void *)0);
    glEnableVertexAttribArray(posLoc);

    // vertex color
    glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(6*sizeof(float)));
    glEnableVertexAttribArray(colorLoc);

    // vertex texture
    glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(normLoc);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}


void Wall::update(glm::vec3& pos, glm::vec3& scale, glm::vec4& rot){
    if(is_static){
        std::cerr << "Tried to update a Static object of type Wall" << std::endl;
        return;
    }
    position = pos;
    this->scale = scale;
    rotation = rot;
}

void Wall::set_lights(AmbientLight& ambient_light, std::vector<PointLight> &pointLights, std::vector<SpotLight> &spotLights){
    // This function remains the same
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