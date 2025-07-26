#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "Shader.h"

#include <iostream>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define NUM 2


class Engine{
public:
    
    int init();
    void render_loop();

private:
    GLFWwindow * window;

    // buffers for shaders
    unsigned int VBO[NUM];

    // hardcoded vertices (for the moment)
    float vertices[NUM][24] = {{
        0.0f,  0.5f, 0.0f, 1.f, .0f, .0f, // top right
        0.0f, -0.5f, 0.0f, .0f, 1.f, .0f, // bottom right
       -0.5f, -0.5f, 0.0f, .0f, .0f, 1.f,  // bottom left
       -0.5f,  0.5f, 0.0f, 1.f, 1.f, 1.f // top left 
      }, 
      {
        0.5f,  0.5f, 0.0f, 1.f, .0f, .0f, // top right
        0.5f, -0.5f, 0.0f, .0f, 1.f, .0f, // bottom right
        0.0f, -0.5f, 0.0f, .0f, .0f, 1.f,  // bottom left
        0.0f,  0.5f, 0.0f, 1.f, 1.f, 1.f   // top left 
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

    float vertices_c[24] = {
        0.5f,  0.5f, 0.0f, 1.f, .0f, .0f, // top right
        0.0f, -0.5f, 0.0f, .0f, 1.f, .0f, // bottom right
       -0.0f, -0.5f, 0.0f, .0f, .0f, 1.f,  // bottom left
       -0.0f,  0.5f, 0.0f, 1.f, 1.f, 1.f   // top left 
   };

   unsigned int indices_c[6] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    }; 

    Shader * shaders[NUM];

    // Vertex Attribute Object
    unsigned int VAO[NUM];

    // Element Buuffer Object
    unsigned int EBO[NUM];

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void process_input();
    void init_buffers();
    void init_shaders();
    void init_VAO();
    void draw();
};