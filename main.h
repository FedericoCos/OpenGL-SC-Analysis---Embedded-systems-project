#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "shader.h"
#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define NUM 2
#define CUBES 10


class Engine{
public:
    
    int init();
    void render_loop();

private:
    GLFWwindow * window;

    // buffers for shaders
    unsigned int VBO[NUM];

    float cube[192] = {
    // Front face (z = 0)
    0.5f,  0.5f, 0.5f,   1.f, 1.f, 1.f,   1.f, 1.f, // 0 top right
    0.5f, -0.5f, 0.5f,   1.f, 1.f, 1.f,   1.f, 0.f, // 1 bottom right
-0.5f, -0.5f, 0.5f,   1.f, 1.f, 1.f,   0.f, 0.f, // 2 bottom left
-0.5f,  0.5f, 0.5f,   1.f, 1.f, 1.f,   0.f, 1.f, // 3 top left

    // Back face (z = 1)
    0.5f,  0.5f, -.5f,   1.f, 1.f, 1.f,   0.f, 1.f, // 4 top right
    0.5f, -0.5f, -.5f,   1.f, 1.f, 1.f,   0.f, 0.f, // 5 bottom right
-0.5f, -0.5f, -.5f,   1.f, 1.f, 1.f,   1.f, 0.f, // 6 bottom left
-0.5f,  0.5f, -.5f,   1.f, 1.f, 1.f,   1.f, 1.f, // 7 top left

    // Left face (x = -0.5)
-0.5f,  0.5f, -.5f,   1.f, 1.f, 1.f,   1.f, 1.f, // 8 top right
-0.5f, -0.5f, -.5f,   1.f, 1.f, 1.f,   1.f, 0.f, // 9 bottom right
-0.5f, -0.5f, 0.5f,   1.f, 1.f, 1.f,   0.f, 0.f, //10 bottom left
-0.5f,  0.5f, 0.5f,   1.f, 1.f, 1.f,   0.f, 1.f, //11 top left

    // Right face (x = 0.5)
    0.5f,  0.5f, 0.5f,   1.f, 1.f, 1.f,   0.f, 1.f, //12 top left
    0.5f, -0.5f, 0.5f,   1.f, 1.f, 1.f,   0.f, 0.f, //13 bottom left
    0.5f, -0.5f, -.5f,   1.f, 1.f, 1.f,   1.f, 0.f, //14 bottom right
    0.5f,  0.5f, -.5f,   1.f, 1.f, 1.f,   1.f, 1.f, //15 top right

    // Top face (y = 0.5)
    0.5f,  0.5f, -.5f,   1.f, 1.f, 1.f,   1.f, 1.f, //16 top right
    0.5f,  0.5f, 0.5f,   1.f, 1.f, 1.f,   1.f, 0.f, //17 bottom right
-0.5f,  0.5f, 0.5f,   1.f, 1.f, 1.f,   0.f, 0.f, //18 bottom left
-0.5f,  0.5f, -.5f,   1.f, 1.f, 1.f,   0.f, 1.f, //19 top left

    // Bottom face (y = -0.5)
    0.5f, -0.5f, 0.5f,   1.f, 1.f, 1.f,   1.f, 0.f, //20 bottom right
    0.5f, -0.5f, -.5f,   1.f, 1.f, 1.f,   1.f, 1.f, //21 top right
-0.5f, -0.5f, -.5f,   1.f, 1.f, 1.f,   0.f, 1.f, //22 top left
-0.5f, -0.5f, 0.5f,   1.f, 1.f, 1.f,   0.f, 0.f, //23 bottom left
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

    unsigned int cVBO; // Buffer for cube indices, just a single one
    unsigned int cEBO; // same as above
    unsigned int cVAO;

    glm::vec3 tr[CUBES];
    glm::vec3 sc[CUBES];
    glm::vec3 rot[CUBES];

    Camera * cam;

    glm::mat4 projection;

    // hardcoded vertices (for the moment)
    float vertices[NUM][32] = {{
        0.0f,  0.5f, 0.0f, 1.f, .0f, .0f, 2.0f, 2.0f, // top right
        0.0f, -0.5f, 0.0f, .0f, 1.f, .0f, 2.0f, 0.0f, // bottom right
       -0.5f, -0.5f, 0.0f, .0f, .0f, 1.f, 0.0f, 0.0f,  // bottom left
       -0.5f,  0.5f, 0.0f, 1.f, 1.f, 1.f, 0.0f, 2.0f // top left 
      }, 
      {
        0.5f,  0.5f, 0.0f, 1.f, .0f, .0f, 1.0f, 1.0f, // top right
        0.5f, -0.5f, 0.0f, .0f, 1.f, .0f, 1.0f, 0.0f, // bottom right
        0.0f, -0.5f, 0.0f, .0f, .0f, 1.f, 0.0f, 0.0f,  // bottom left
        0.0f,  0.5f, 0.0f, 1.f, 1.f, 1.f, 0.0f, 1.0f   // top left 
      } 
   };

   unsigned int indices[NUM][6] = {{  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
   },{
        0, 1, 3,
        1, 2, 3
   }
    }; 

    float mix_val = 0.2;

    Shader * shaders[NUM];

    // Vertex Attribute Object
    unsigned int VAO[NUM];

    // Element Buuffer Object
    unsigned int EBO[NUM];

    // texture placeholder
    unsigned int texture[2];

    // transformation matrix
    glm::mat4 trans[NUM];

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void process_input();
    void init_buffers();
    void init_shaders();
    void init_VAO();
    void init_textures();
    void draw();
};