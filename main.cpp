#include "main.h"
#include "math.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void Engine::framebuffer_size_callback(GLFWwindow* window, int w, int h){
    glViewport(0, 0, w, h);
}

int Engine::init(){
    bool saveCsv = true;
    tracker.init(saveCsv, 100, "scene_stats.csv");

    // INIZIALIZZAZIONE FINESTRA

    // GLFW initialization
    glfwInit();
    // First specify version of OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // first argument is what we want to configure
                                                   // second is the value for that option
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // load open gl functions with no back compatibility

    window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "OpenGL", NULL, NULL);
    if(window == NULL){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // GLAD initialization
    /**
     * GLAD contains alla the fucntions not available in system OpenGL Library
     */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT); // lower-left corner of the window, and dimension
    width = WIN_WIDTH;
    height = WIN_HEIGHT;
    glfwSetFramebufferSizeCallback(window, Engine::framebuffer_size_callback);


    stbi_set_flip_vertically_on_load(true);
    is_imgui = false;

    // Camera initialization
    glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 tar = glm::vec3(0.0f, 0.0f, -1.0f);
    cam = new Camera(pos, tar, 30.0f, 2.5f);

    init_scene_objects();
    init_light_resources();
    init_shadow_resources();

    // To disable vsync
    glfwSwapInterval(0);

    return 0;
}

void Engine::process_input(){
    // Refreshing the input
    right_input.x = 0;
    right_input.y = 0;
    left_input.x = 0;
    left_input.y = 0;

    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        right_input.x = 1;
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        right_input.x = -1;
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        right_input.y = -1;
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        right_input.y = 1;
    }

    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
        left_input.x = 1;
    }
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
        left_input.x = -1;
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
        left_input.y = -1;
    }
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
        left_input.y = 1;
    }
    
}

void Engine::init_scene_objects(){
    // WALL INITIALIZATION
    std::cout << "Initializing Walls..." << std::endl;
    wall_scale = wall_scale_factor * wall_scale;
    for(size_t i = 0; i < WALLS; i++){
        walls[i] = Wall("shaders/vertex_wall.glsl", "shaders/fragment_wall.glsl", true);
        wall_positions[i] = wall_scale_factor * wall_positions[i];
        walls[i].init(wall_positions[i], wall_scale, wall_rotation[i]);
    }
    std::cout << "Wall initialized!" << std::endl;

    // LIGHT CUBE INITIALIZATION
    std::cout << "Initializing Cube lights..." << std::endl;
    cube_positions.resize(num_lights);
    cube_colors.resize(num_lights);
    cubes.resize(num_lights);
    glm::vec3 s(2.f, 2.f, 2.f);
    glm::vec4 r(1.f, 1.f, 1.f, .0f);
    for(size_t i = 0; i < num_lights; i++){
        glm::vec3 dir = glm::sphericalRand(1.0f); 
        float dist = glm::linearRand(min_cube_spread, max_cube_spread);

        cube_positions[i] = dist * dir;

        cube_colors[i] = glm::vec4(
            glm::linearRand(0.1f, 1.0f),
            glm::linearRand(0.1f, 1.0f),
            glm::linearRand(0.1f, 1.0f),
            glm::linearRand(0.5f, 1.0f)
        );

        cubes[i] = Cube("shaders/vertex_light.glsl", "shaders/fragment_light.glsl", true);
        cubes[i].init(cube_positions[i], s, r);
    }
    std::cout << "Cube lights initialized" << std::endl;

    // MODEL INITIALIZATION
    std::cout << "Initializing model..." << std::endl;
    backpack_model = Model("/home/zancanonzanca/Desktop/OpenGL-SC-Analysis---Embedded-systems-project/resources/backpack/backpack.obj");
    backpack_shader = Shader("shaders/vertex_model.glsl", "shaders/fragment_model.glsl");
    std::cout << "Model initialized!" << std::endl;
}

void Engine::init_shadow_resources(){
    std::cout << "Initializing shdow resources..." << std::endl;
    simpleDepthShader = Shader("shaders/vertex_shadow.glsl", "shaders/fragment_shadow.glsl");

    glGenFramebuffers(1, &depthMapFBO);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); 


    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
    std::cout << "Shadow resources Initiliazed!" << std::endl;
}

void Engine::init_light_resources(){
    // Ambient light
    glm::vec3 pos(0.f, 0.f, 0.f);
    glm::vec4 col(1.f, 1.f, 1.f, .1f);
    glm::vec3 vel(0.f, 0.f, 0.f);
    glm::vec3 dir(-1.f, -1.f, 0.f);
    float fact1 = .2f;
    float fact2 = .5f;

    ambientLight = AmbientLight(pos, col, vel, dir, fact1, fact2);

    // pointLights
    pointLights.resize(num_lights);
    for(size_t i = 0; i < num_lights; i++){
        pointLights[i] = PointLight(cube_positions[i], cube_colors[i], vel);
    }

    // spotLights
    spotLights.resize(1);
    col = glm::vec4(1.f, 1.f, 1.f, 0.5f);
    spotLights[0] = SpotLight(cam -> position, col, vel, cam -> front, 17.5, 22.5f);
}

void Engine::render_loop(){
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glEnable(GL_DEPTH_TEST);

    projection = glm::perspective(glm::radians(fov), width * 1.f/height, near_plane, far_plane);

    while(!glfwWindowShouldClose(window))
    {
        tracker.beginFrame();
        glfwGetWindowSize(window, &width, &height);


        float t = (float)glfwGetTime();
        dtime = t - past_time;
        past_time = t;
        process_input();
        cam -> update(right_input, left_input, dtime);

         tracker.setActiveLights(pointLights.size(), spotLights.size());

        // First, let's set the spotligt
        light_projection = glm::perspective(glm::radians(2 * fov), 1.f, 0.1f, 50.f);
        spotLights[0].align_light(cam -> getLightPos(), 
                                    cam -> front, 
                                    cam -> up);
        light_space_matrix = light_projection * spotLights[0].getLookAt();

        tracker.beginShadowPass();
        glCullFace(GL_FRONT);
        shadow_pass();
        glCullFace(GL_BACK);

        tracker.endShadowPass();

        tracker.beginCpuRender();

        glViewport(0, 0, width, height);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        draw();

        tracker.endCpuRender();

        glfwSwapBuffers(window);
        glfwPollEvents(); 

        tracker.endFrame();
        // tracker.printStats();
    }

    glfwTerminate();
}

void Engine::shadow_pass(){
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    simpleDepthShader.use();
    simpleDepthShader.setMatrix("lightSpaceMatrix", light_space_matrix);

    simpleDepthShader.setFloat("near_plane", 0.1f);
    simpleDepthShader.setFloat("far_plane", 50.f);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    // Render Everything from light perspective

    // RENDERING WALLS
    for(int i = 0; i < 5; i++){
        walls[i].draw(simpleDepthShader);
        tracker.countShadowDraw();
    }

    // RENDERING CUBES
    for(size_t i = 0; i < num_lights; i++){
        cubes[i].draw(simpleDepthShader);
         tracker.countShadowDraw();
    }

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));	// it's a bit too big for our scene, so scale it down
    simpleDepthShader.setMatrix("model", model);

    backpack_model.Draw(simpleDepthShader, false);
    tracker.countShadowDraw();

    glBindVertexArray(0);


    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}

void Engine::draw(){
    glm::mat4 view = cam -> viewAtMat();
    glm::vec3 spotPos = cam -> getLightPos();

    Shader current_shader;

    // RENDERING WALLS
    for(size_t i = 0; i < WALLS; i++){
        // First extract shader
        current_shader = walls[i].getShader();
        current_shader.use();

        // Bind texture for shadows
        glActiveTexture(GL_TEXTURE0);
        current_shader.setInt("shadowMap", 0);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        current_shader.setMatrix("lightSpaceMatrix", light_space_matrix);

        // VP matrix
        current_shader.setMatrix("view", view);
        current_shader.setMatrix("projection", projection);

        // Setting lights
        current_shader.setVec3("viewPos", cam->position);
        walls[i].set_lights(ambientLight, pointLights, spotLights);

        // Draw call
        walls[i].draw();
        tracker.countWallDraw(2);
    }

    // RENDERING LIGHTS CUBE
    for(size_t i = 0; i < num_lights; i++){
        current_shader = cubes[i].getShader();
        current_shader.use();

        // Bind texture for shadows
        glActiveTexture(GL_TEXTURE0);
        current_shader.setInt("shadowMap", 0);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        // VP matrix
        current_shader.setMatrix("view", view);
        current_shader.setMatrix("projection", projection);

        current_shader.setVec4("color", cube_colors[i]);

        cubes[i].draw();
        tracker.countPointLightDraw(12);
    }

    // RENDERING MODEL
    backpack_shader.use();
    backpack_shader.setMatrix("projection", projection);
    backpack_shader.setMatrix("view", view);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));	// it's a bit too big for our scene, so scale it down
    backpack_shader.setMatrix("model", model);

    backpack_shader.setMatrix("lightSpaceMatrix", light_space_matrix);

    backpack_model.set_lights(backpack_shader, ambientLight, pointLights, spotLights);
    backpack_model.Draw(backpack_shader, true, depthMap);

    tracker.countModelDraw(backpack_model.triangle_count());

    glBindVertexArray(0);
}


int main(int argc, char * argv[]){
    Engine engine;
    if(engine.init()){
        return -1;
    }

    engine.render_loop();

    return 0;
}



