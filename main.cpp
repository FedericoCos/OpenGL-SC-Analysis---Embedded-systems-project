#include "main.h"

int Engine::init(){
    // Initialize SDL and create a window
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

    tracker.init();

    init_cubes();
    init_camera();

    init_shaders();
    init_buffers();

    init_textures();

    // Setting the projection matrix
    projection = glm::perspective(glm::radians(45.f), 800.f/600.f, 0.1f, 100.f);

    eglSwapInterval(eglDisplay, 0);

    return 0;
}

void Engine::init_buffers(){
    // Vertex buffer object for cubes
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
    tracker.trackVramAllocation(sizeof(cube));

    // vertex position
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void *)0);
    glEnableVertexAttribArray(posLoc);

    // vertex color
    glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3*sizeof(float)));
    glEnableVertexAttribArray(colorLoc);

    // vertex texture
    glVertexAttribPointer(textureLoc, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(textureLoc);

    // cubes indices
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);
    tracker.trackVramAllocation(sizeof(cube_indices));

}

void Engine::init_camera(){
    glm::vec3 pos(0.f, 0.f, 3.f);
    glm::vec3 tar(0.f, 0.f, -1.f);
    cam = Camera(pos, tar, 30.f, 2.5f);
}

void Engine::init_shaders(){
    program = Shader::loadOrCreateProgram("my_shader.bin", vertexShaderSource, fragmentShaderSource);

    // Getting the locations of uniforms and attributes in the shader
    posLoc = glGetAttribLocation(program, "vPosition");
    colorLoc = glGetAttribLocation(program, "vColor");
    rotAxLoc = glGetUniformLocation(program, "rotAxis");
    modelLoc = glGetUniformLocation(program, "model");
    viewLoc = glGetUniformLocation(program, "view");
    projectionLoc = glGetUniformLocation(program, "projection");
    timeLoc = glGetUniformLocation(program, "time");
    textureLoc = glGetAttribLocation(program, "vTex");
    text1Loc = glGetUniformLocation(program, "texture1");
    text2Loc = glGetUniformLocation(program, "texture2");
}

void Engine::init_cubes(){
    tr.resize(CUBES);
    sc.resize(CUBES);
    rot.resize(CUBES);
    trans.resize(CUBES);

    // Random variables for cubes
    for(int i = 0; i < CUBES; i++){
        tr[i] = std::max((rand() % 1000 * 0.1f), 0.5f) * glm::normalize(glm::vec3(rand() % 10 - 5, rand() % 10 - 5, rand() % 10 - 5));
        sc[i] = glm::vec3(std::max(rand() % 10 * 0.1f, 0.1f));
        rot[i] = glm::normalize(glm::vec3(std::min((rand() % 10 - 5) * 0.1f, 0.1f), std::min((rand() % 10 - 5) * 0.1f, 0.1f), std::min((rand() % 10 - 5) * 0.1f, 0.1f)));
    }
}

void Engine::init_textures(){
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char *data = stbi_load("textures/wall.jpg", &width, &height, &nrChannels, 0);
    if(!data){
        std::cout << "Failed to load texture" << std::endl;
        exit(0);
    }

    glGenTextures(2, texture);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    tracker.countTextureBind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    tracker.countTextureBind();
    data = stbi_load("textures/awesomeface.png", &width, &height, &nrChannels, 0);
    if(data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
}

void Engine::render_loop(){
    running = true;

    while(running){
        tracker.beginFrame();

        float temp = (float)SDL_GetTicks() / 1000.f;
        dtime = temp - last;
        last = temp;

        process_input();

        draw();

        eglSwapBuffers(eglDisplay, eglSurface);

        tracker.endFrame();
        tracker.printStats();
    }
    
    // Cleanup
    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(eglDisplay, eglContext);
    eglDestroySurface(eglDisplay, eglSurface);
    eglTerminate(eglDisplay);
    SDL_DestroyWindow(window);
    SDL_Quit();
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
    cam.update(right_input, left_input, dtime);
}

void Engine::draw(){
        tracker.beginCpuRender();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);
        tracker.countShaderBind();

        // --- RENDER ---

        // Position attribute
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        
        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

        // Set uniforms common to all cubes
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cam.viewAtMat()));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Texture binding
        glUniform1i(text1Loc, 0);
        glUniform1i(text2Loc, 1);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture[0]);
        tracker.countTextureBind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture[1]);
        tracker.countTextureBind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);


        float r = (float)SDL_GetTicks() / 1000.f; // for rotation
        glUniform1f(timeLoc, r);

        for (int i = 0; i < cubes; i++) {
            trans[i] = glm::mat4(1.0f);
            trans[i] = glm::scale(trans[i], sc[i]);
            trans[i] = glm::translate(trans[i], tr[i]);

            glUniform3fv(rotAxLoc, 1, glm::value_ptr(rot[i]));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(trans[i]));

            // Draw the cube
            tracker.countDrawCall();
            tracker.countTriangles(12);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0);
        }

        tracker.endCpuRender();
}



int main(int argc, char * argv[]){
    Engine engine;
    if(argc > 1)
        engine.set_cubes_num(std::stoi(argv[1]));

    if(engine.init()){
        return -1;
    };

    engine.render_loop();


    return 0;
}
