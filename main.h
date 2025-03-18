#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "Shader.h"

#include <iostream>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600


class Engine{
public:
    
    int init();
    void render_loop();

private:
    GLFWwindow * window;

    // buffers for shaders
    unsigned int VBO;

    // hardcoded vertices (for the moment)
    float vertices[24] = {
        0.5f,  0.5f, 0.0f, 1.f, .0f, .0f, // top right
        0.5f, -0.5f, 0.0f, .0f, 1.f, .0f, // bottom right
       -0.5f, -0.5f, 0.0f, .0f, .0f, 1.f,  // bottom left
       -0.5f,  0.5f, 0.0f, 1.f, 1.f, 1.f   // top left 
   };

   unsigned int indices[6] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    }; 

    Shader *mainShader;

    // Vertex Attribute Object
    unsigned int VAO;

    // Element Buuffer Object
    unsigned int EBO;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void process_input();
    void init_buffers();
    void init_shaders();
    void init_VAO();
    void draw();
};