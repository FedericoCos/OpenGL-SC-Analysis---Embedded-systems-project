#include "glad/glad.h"
#include "GLFW/glfw3.h"

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
    float vertices[12] = {
        0.5f,  0.5f, 0.0f,  // top right
        0.5f, -0.5f, 0.0f,  // bottom right
       -0.5f, -0.5f, 0.0f,  // bottom left
       -0.5f,  0.5f, 0.0f   // top left 
   };

   unsigned int indices[6] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    }; 

    // hardcoded vertex shader for now
    const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0"; 

    // vertex shader related variables
    unsigned int vertexShader;

    // hardcoded fragment shder for the moment
    const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
            "FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\0";
    
    // fragment shader related variables
    unsigned int fragmentShader;

    // Shader program object
    unsigned int shaderProgram;

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