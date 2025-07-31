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

    return 0;
}


void Engine::render_loop(){
    bool running = true;
    GLuint program = Shader::createProgram(vertexShaderSource, fragmentShaderSource);
    GLint posLoc = glGetAttribLocation(program, "vPosition");
    GLint colorLoc = glGetAttribLocation(program, "vColor");
    GLint mvpLoc = glGetUniformLocation(program, "u_mvpMatrix");
    
    while(running){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT){
                running = false;
            }
        }

        glClearColor(0.2f, 0.4f, 0.6f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);

        // --- MATRIX CALCULATIONS ---
        glm::mat4 mat(1.f);
        mat = glm::translate(mat, glm::vec3(0.f, 0.f, -5.f));
        mat = glm::rotate(mat, (float)SDL_GetTicks() / 1000.f, glm::vec3(1.f, 0.f, 1.f));
         mat = cam.viewAtMat() * mat;
        mat = glm::perspective(glm::radians(45.f), 800.f/600.f, 0.1f, 100.f) * mat;
        

        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mat));

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

        // Draw the cube
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0);

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



int main(){
    Engine engine;

    if(engine.init()){
        return -1;
    };

    engine.render_loop();


    return 0;
}
