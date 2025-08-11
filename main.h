#pragma once

#include "compute_shader.h"
#include "helpers.h"

#include "shader.h"

#include "perfTracker.h"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

class Engine{
public:
    
    int init(int cubes, bool imgui, bool save);
    void render_loop();

private:

    GLuint wallTex;
    GLuint compTex;
    int wallW = 0, wallH = 0, wallBpp = 0;

    ComputeShader* mengerCS;
    Shader screen_shader;
    GLuint quadVAO, quadVBO;
    GLFWwindow * window;
    float past_time = 0;
    float dtime;
    PerfTracker tracker;

    int width;
    int height;

    struct Light
    {
        glm::vec3 position;
        glm::vec3 color;
    };

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void process_input();
    void init_shaders();
    void init_VAO();
    void init_textures();

};