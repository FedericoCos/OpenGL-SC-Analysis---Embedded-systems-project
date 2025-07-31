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

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &cbo);
    glBindBuffer(GL_ARRAY_BUFFER, cbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glm::vec3 pos(0.f, 0.f, 3.f);
    glm::vec3 tar(0.f, 0.f, -1.f);
    cam = Camera(pos, tar, 30.f, 2.5f);

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

    eglSwapInterval(eglDisplay, 0);

    return 0;
}


void Engine::render_loop(){
    bool running = true;
    GLuint program = Shader::createProgram(vertexShaderSource, fragmentShaderSource);
    GLint posLoc = glGetAttribLocation(program, "vPosition");
    GLint colorLoc = glGetAttribLocation(program, "vColor");
    GLint modelLoc = glGetUniformLocation(program, "model");
    GLint viewLoc = glGetUniformLocation(program, "view");
    GLint projectionLoc = glGetUniformLocation(program, "projection");

    int frames = 0;
    Uint32 lastTime = SDL_GetTicks();
    
    while(running){
        Uint32 currentTime = SDL_GetTicks();
        frames++;
        if (currentTime - lastTime >= 1000) {
            std::cout << "FPS: " << frames << std::endl;
            frames = 0;
            lastTime = currentTime;
        }
        float temp = (float)SDL_GetTicks() / 1000.f;
        dtime = temp - last;
        last = temp;
        process_input();

        glClearColor(0.2f, 0.4f, 0.6f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);

        // --- RENDER ---

        // Position attribute
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(posLoc);

        // Color attribute
        glBindBuffer(GL_ARRAY_BUFFER, cbo);
        glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(colorLoc);
        
        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cam.viewAtMat()));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(glm::perspective(glm::radians(45.f), 800.f/600.f, 0.1f, 100.f)));

        float r = (float)SDL_GetTicks() / 1000.f;
        // --- MATRIX CALCULATIONS ---
        for (int i = 0; i < cubes; i++) {
            trans[i] = glm::mat4(1.0f);
            trans[i] = glm::scale(trans[i], sc[i]);
            trans[i] = glm::translate(trans[i], tr[i]);
            trans[i] = glm::rotate(trans[i], r, rot[i]);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(trans[i]));

            // Draw the cube
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0);
        }

        eglSwapBuffers(eglDisplay, eglSurface);
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
            exit(0);
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            exit(0);
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



int main(){
    Engine engine;

    if(engine.init()){
        return -1;
    };

    engine.render_loop();


    return 0;
}
