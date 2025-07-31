// GLES
#include <GLES2/gl2.h>
#include <EGL/egl.h>

// EGL
#include <EGL/eglext.h>

// SDL2
#include <SDL2/SDL.h> // For Events
#include <SDL2/SDL_syswm.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <iostream>
#include <vector>
#include <cmath>

#include "shader.h"
#include "camera.h"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define CUBES 1000000

class Engine{
public:
    int init();
    void render_loop();

private:
    SDL_Window* window;
    bool fullscreen = false;
    float dtime, last;

    EGLDisplay eglDisplay;
    EGLSurface eglSurface;
    EGLContext eglContext;

    SDL_Event event; // for input handling

    Camera cam;
    glm::vec2 right_input;
    glm::vec2 left_input;

    float vertices[24] = {
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f
    };

    float colors[32] = {
        0.0f,  1.0f,  0.0f,  1.0f,
        0.0f,  1.0f,  0.0f,  1.0f,
        1.0f,  0.5f,  0.0f,  1.0f,
        1.0f,  0.5f,  0.0f,  1.0f,
        1.0f,  0.0f,  0.0f,  1.0f,
        1.0f,  0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,  1.0f,
        1.0f,  0.0f,  1.0f,  1.0f
    };

    GLubyte indices[36] = {
        0, 4, 5, 0, 5, 1,
        1, 5, 6, 1, 6, 2,
        2, 6, 7, 2, 7, 3,
        3, 7, 4, 3, 4, 0,
        4, 7, 6, 4, 6, 5,
        3, 0, 1, 3, 1, 2
    };

    int cubes = 1000;
    std::vector<glm::vec3> tr;
    std::vector<glm::vec3> sc;
    std::vector<glm::vec3> rot;
    std::vector<glm::mat4> trans;

    const char* vertexShaderSource = R"(
        #version 100
        attribute vec4 vPosition;
        attribute vec4 vColor;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        varying vec4 fragColor;

        void main() {
            gl_Position = projection*view*model * vPosition;
            fragColor = vColor;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 100
        precision mediump float;
        
        varying vec4 fragColor;

        void main() {
            gl_FragColor = fragColor;
        }
    )";

    GLuint vbo, cbo, ibo;


    // ----------------- FUNCTIONS
    void process_input();


};