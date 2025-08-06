#pragma once
#include "helpers.h"

#include "shader.h"
#include "camera.h"


#include "perfTracker.h"
#include "model.h"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define NUM 2
#define CUBES 1000000


class Engine{
public:
    
    int init(int cubes, bool imgui, bool save);
    void render_loop();

private:
    GLFWwindow * window;
    float past_time = 0;
    float dtime;
    PerfTracker tracker;
    bool is_imgui = true;

    float wall_vertices[36] = {
        // positions           // normals            // colors
        1.0f,  1.0f, 0.0f,    0.0f, 0.0f,1.0f,    0.5f, 0.5f, 0.5f,
        -1.0f,  1.0f, 0.0f,    0.0f, 0.0f,1.0f,    0.5f, 0.5f, 0.5f,
        -1.0f, -1.0f, 0.0f,    0.0f, 0.0f,1.0f,    0.5f, 0.5f, 0.5f,
        1.0f, -1.0f, 0.0f,    0.0f, 0.0f,1.0f,    0.5f, 0.5f, 0.5f
    };

    unsigned int wall_indices[6] = {
        0, 1, 2,
        0, 2, 3
    };
    glm::vec3 wall_positions[6] = {
        {0.0f, 0.0f, -1.0f},  // Front
        {-1.0f, 0.0f,  0.0f}, // Left
        { 1.0f, 0.0f,  0.0f}, // Right
        {0.0f,  1.0f,  0.0f}, // Top
        {0.0f, -1.0f,  0.0f}, // Bottom
        {0.0f, 0.0f,  1.0f}   // Back
    };

    glm::vec4 wall_rotation[6] = {
        {0.0f, 1.0f, 0.0f,   0.0f},   // Front → facing in
        {0.0f, 1.0f, 0.0f,  90.0f},   // Left → facing in
        {0.0f, 1.0f, 0.0f, -90.0f},   // Right → facing in
        {1.0f, 0.0f, 0.0f, 90.0f},   // Top → facing in
        {1.0f, 0.0f, 0.0f,  -90.0f},   // Bottom → facing in
        {0.0f, 1.0f, 0.0f, 180.0f}    // Back → facing in
    };

    float wall_scale_factor = 10.f;
    glm::vec3 wall_scale = {1.f, 1.f, 1.f};

    unsigned int wallVAO;
    unsigned int wallVBO;
    unsigned int wallEBO;
    Shader wall_shader;

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

    std::vector<glm::vec3> light_positions;

    std::vector<glm::vec3> light_color;

    glm::vec3 light_scale = {.2f, .2f, .2f};

    int num_lights = 6;

    float min_light_dis = 10.f;
    float max_light_dis = 20.f;

    unsigned int lightVAO;
    unsigned int lightVBO;
    unsigned int lightEBO;
    Shader light_shader;


    Camera * cam;
    glm::vec2 left_input;
    glm::vec2 right_input;

    // Projection varibales
    float fov = 45.f;
    float near_plane = 0.1f;
    float far_plane = 1000.f;
    int width;
    int height;

    glm::mat4 projection;

    Model backpack_model;
    Shader backpack_shader;
    

    // texture placeholder
    unsigned int texture[2];

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void process_input();
    void init_buffers();
    void init_shaders();
    void init_VAO();
    void init_textures();


    void draw();
    void draw_imgui();
};