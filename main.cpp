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
    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    window = SDL_CreateWindow("CUBES ES", 0, 0, WIN_WIDTH, WIN_HEIGHT,
    fullscreen ? (SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN) : SDL_WINDOW_OPENGL);

    // Initialize EGL
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(eglDisplay, nullptr, nullptr);

    // Configure EGL
    const EGLint configAttributes[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, // Request GLES 2.0
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_BUFFER_SIZE, 16,
        EGL_STENCIL_SIZE,   0,
        EGL_NONE
    };

    EGLConfig eglConfig;
    EGLint numConfig;
    eglChooseConfig(eglDisplay, configAttributes, &eglConfig, 1, &numConfig);

    SDL_SysWMinfo sysInfo;
    SDL_VERSION(&sysInfo.version); // Set SDL version
    SDL_GetWindowWMInfo(window, &sysInfo);
    // The native window handle is platform-specific
    EGLNativeWindowType nativeWindow = (EGLNativeWindowType)sysInfo.info.x11.window;

    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, nativeWindow, NULL);

    // Create EGL Context
    const EGLint contextAttributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };

    eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttributes);

    // Make the Context Current
    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
    
    glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);

    glClearDepthf(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

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
    eglSwapInterval(eglDisplay, 0);

    return 0;
}

void Engine::process_input() {
    // Process all SDL events first
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
            return;
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            running = false;
            return;
        }
    }

    // Get the current keyboard state
    const Uint8* state = SDL_GetKeyboardState(NULL);

    // Reset inputs
    right_input = {0.f, 0.f};
    left_input  = {0.f, 0.f};

    // --- Right hand (WASD) ---
    if (state[SDL_SCANCODE_W]) right_input.x = 1.f;
    if (state[SDL_SCANCODE_S]) right_input.x = -1.f;
    if (state[SDL_SCANCODE_A]) right_input.y = -1.f;
    if (state[SDL_SCANCODE_D]) right_input.y =  1.f;

    // --- Left hand (Arrow keys) ---
    if (state[SDL_SCANCODE_UP])    left_input.x = 1.f;
    if (state[SDL_SCANCODE_DOWN])  left_input.x = -1.f;
    if (state[SDL_SCANCODE_LEFT])  left_input.y = -1.f;
    if (state[SDL_SCANCODE_RIGHT]) left_input.y =  1.f;

    // Update camera or game logic with smoothed input
    cam -> update(right_input, left_input, dtime);
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
    std::cout << "Initializing shadow resources..." << std::endl;
    simpleDepthShader = Shader("shaders/vertex_shadow.glsl", "shaders/fragment_shadow.glsl");

    glGenFramebuffers(1, &depthMapFBO);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    // --- FIX: Use GLES 2.0 compatible formats for depth texture ---
    // GL_DEPTH_COMPONENT16 is a guaranteed core format in GLES 2.0
    // The type should be UNSIGNED_SHORT to match.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Use GL_CLAMP_TO_EDGE for shadow maps to avoid artifacts at the edges
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

    // --- FIX: Remove desktop-only GL calls ---
    // glDrawBuffer and glReadBuffer are not in GLES 2.0.
    // The behavior is implicit when only a depth buffer is attached.
    /* glDrawBuffer(GL_NONE); */
    /* glReadBuffer(GL_NONE); */

    // Check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    std::cout << "Shadow resources Initialized!" << std::endl;
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
    projection = glm::perspective(glm::radians(fov), WIN_WIDTH * 1.f/WIN_HEIGHT, near_plane, far_plane);
    while(running)
    {
        tracker.beginFrame();

        float t = (float)SDL_GetTicks() / 1000.f;
        dtime = t - past_time;
        past_time = t;
        process_input();

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

        glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        draw();

        tracker.endCpuRender();

        eglSwapBuffers(eglDisplay, eglSurface);

        tracker.endFrame();
        tracker.printStats();
    }


    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(eglDisplay, eglContext);
    eglDestroySurface(eglDisplay, eglSurface);
    eglTerminate(eglDisplay);
    SDL_DestroyWindow(window);
    SDL_Quit();
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



    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}

void Engine::draw(){
    glm::mat4 view = cam -> viewAtMat();
    // This was missing, but needed for the spotlight
    // glm::vec3 spotPos = cam -> getLightPos();

    // --- FIX: Use a reference to the shader to avoid copying ---
    // This is more efficient and safer.
    // Shader& current_shader = walls[0].getShader(); // Example, will be set in loop

    // RENDERING WALLS
    for(size_t i = 0; i < WALLS; i++){
        // Get a reference to the wall's shader
        Shader& current_shader = walls[i].getShader();
        current_shader.use();

        // Bind texture for shadows
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        current_shader.setInt("shadowMap", 0);


        current_shader.setMatrix("lightSpaceMatrix", light_space_matrix);

        // VP matrix
        current_shader.setMatrix("view", view);
        current_shader.setMatrix("projection", projection);

        // Setting lights
        current_shader.setVec3("viewPos", cam->position);
        walls[i].set_lights(ambientLight, pointLights, spotLights);

        // The draw call itself now handles binding and vertex attributes
        walls[i].draw();
        tracker.countWallDraw(2);
    }

    // RENDERING LIGHTS CUBE
    for(size_t i = 0; i < num_lights; i++){
        Shader& current_shader = cubes[i].getShader();
        current_shader.use();

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
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
    backpack_shader.setMatrix("model", model);

    backpack_shader.setMatrix("lightSpaceMatrix", light_space_matrix);
    backpack_shader.setVec3("viewPos", cam->position); // The model shader also needs the view position

    backpack_model.set_lights(backpack_shader, ambientLight, pointLights, spotLights);
    backpack_model.Draw(backpack_shader, true, depthMap);
    tracker.countModelDraw(backpack_model.triangle_count());
}


int main(int argc, char * argv[]){
    Engine engine;
    if(engine.init()){
        return -1;
    }

    engine.render_loop();

    return 0;
}



