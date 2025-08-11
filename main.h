#pragma once
#include "helpers.h"

#include "shader.h"
#include "camera.h"


#include "perfTracker.h"
#include "model.h"

// Lights classes
#include "spotlight.h"
#include "pointlight.h"
#include "ambientlight.h"

// Wall Object
#define WALLS 5 // NUMBER OF WALLS
#include "wall.h"

// CUBE Object
#include "cube.h"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600


class Engine{
public:
    
    int init();
    void render_loop();

private:
    GLFWwindow * window;
    float past_time = 0;
    float dtime;
    PerfTracker tracker;
    bool is_imgui = true;

    // CAMERA VARIABLES
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

    // WALLS VARIABLES
    Wall walls[WALLS];

    glm::vec3 wall_positions[WALLS] = {
        {0.0f, 0.0f, -1.0f},  // Front
        {-1.0f, 0.0f,  0.0f}, // Left
        { 1.0f, 0.0f,  0.0f}, // Right
        {0.0f,  1.0f,  0.0f}, // Top
        {0.0f, -1.0f,  0.0f}, // Bottom
        // {0.0f, 0.0f,  1.0f}   // Back
    };

    glm::vec4 wall_rotation[WALLS] = {
        {0.0f, 1.0f, 0.0f,   0.0f},   // Front → facing in
        {0.0f, 1.0f, 0.0f,  90.0f},   // Left → facing in
        {0.0f, 1.0f, 0.0f, -90.0f},   // Right → facing in
        {1.0f, 0.0f, 0.0f, 90.0f},   // Top → facing in
        {1.0f, 0.0f, 0.0f,  -90.0f},   // Bottom → facing in
    };

    float wall_scale_factor = 15.f;
    glm::vec3 wall_scale = {1.f, 1.f, 1.f};

    // CUBE LIGHTS VARIABLES
    std::vector<Cube> cubes;
    int num_lights = 6;
    std::vector<glm::vec3> cube_positions;
    std::vector<glm::vec4> cube_colors;
    float min_cube_spread = 10.f;
    float max_cube_spread = 15.f;

    // LIGHTS VARIABLES
    AmbientLight ambientLight;
    std::vector<PointLight> pointLights;
    std::vector<SpotLight> spotLights;

    Model backpack_model;
    Shader backpack_shader;

    // For shadow
    unsigned int depthMapFBO;
    unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    unsigned int depthMap;
    Shader simpleDepthShader;

    // For spotlight
    glm::mat4 light_projection;
    glm::mat4 light_view;
    glm::mat4 light_space_matrix;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void process_input();

    void init_scene_objects();
    void init_shadow_resources();
    void init_light_resources();


    void draw();
    void shadow_pass();

};