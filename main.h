#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    glm::mat4 trans;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void process_input();
    void init_buffers();
    void init_shaders();
    void init_VAO();
    void init_textures();
    void draw();
};