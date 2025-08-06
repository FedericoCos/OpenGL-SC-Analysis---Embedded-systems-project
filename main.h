// GLES
#include <GLES2/gl2.h>
#include <EGL/egl.h>

// EGL
#include <EGL/eglext.h>

// SDL2
#include <SDL2/SDL.h> // For Events
#include <SDL2/SDL_syswm.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <cmath>

#include "shader.h"
#include "camera.h"
#include "perfTracker.h"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define CUBES 1000000

class Engine{
public:
    int init();
    void render_loop();
    void set_cubes_num(int num){
        cubes = std::min(num, CUBES);
    }

private:
    SDL_Window* window;
    bool fullscreen = false;
    bool running = false;
    float dtime, last;

    EGLDisplay eglDisplay;
    EGLSurface eglSurface;
    EGLContext eglContext;

    SDL_Event event; // for input handling

    Camera cam;
    glm::vec2 right_input;
    glm::vec2 left_input;

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
    GLubyte cube_indices[36] = {
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

    int cubes = 1000;
    std::vector<glm::vec3> tr;
    std::vector<glm::vec3> sc;
    std::vector<glm::vec3> rot;
    std::vector<glm::mat4> trans;

    glm::mat4 projection;

    unsigned int texture[2];

    const char* vertexShaderSource = R"(
        #version 100

        attribute vec4 vPosition;
        attribute vec4 vColor;
        attribute vec2 vTex;

        uniform vec3 rotAxis;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        uniform float time;

        varying vec4 fragColor;
        varying vec2 textCoord;


        mat4 rotationMatrix(float angle, vec3 axis) {
            float s = sin(angle);
            float c = cos(angle);
            float oc = 1.0 - c;

            return mat4(
                oc * axis.x * axis.x + c,          oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c,          oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0
            );
        }

        void main() {
            mat4 rotation = rotationMatrix(time, rotAxis);
            gl_Position = projection*view*model*rotation * vPosition;
            fragColor = vColor;
            textCoord = vTex;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 100
        precision mediump float;
        
        varying vec4 fragColor;
        varying vec2 textCoord;

        uniform sampler2D texture1;
        uniform sampler2D texture2;

        void main() {
            gl_FragColor = mix(texture2D(texture1, textCoord),
                   texture2D(texture2, textCoord),
                   0.2); 
        }
    )";

    GLuint vbo, cbo, ibo, rbo, tbo; // Various buffers
    GLuint program;
    GLint posLoc;
    GLint colorLoc;
    GLint textureLoc;
    GLint rotAxLoc;
    GLint modelLoc;
    GLint viewLoc;
    GLint projectionLoc;
    GLint timeLoc;
    GLint text1Loc;
    GLint text2Loc;

    PerfTracker tracker;

    // ——— Compute‐pass state ——————————————————————————
    GLuint computeProg;    // program for compute‐as‐fragment
    GLuint blitProg;      // program for blitting compute output
    GLuint quadVBO, quadVAO;// fullscreen quad
    GLuint compTex, compFBO;// texture & FBO for compute output
    GLint computePosLoc; // attribute location for "aPos" fullscreen quad
    GLint blitPosLoc;   // attribute location for "aPos" in blit quad

    // Helper to init compute pipeline & resources
    void init_compute();
    // Call every frame to run the “compute shader”
    void compute_pass(float time);

    // ----------------- FUNCTIONS
    void process_input();
    void init_buffers();
    void init_cubes();
    void init_camera();
    void init_shaders();
    void init_textures();

    void draw();
};