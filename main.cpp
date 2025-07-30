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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &cbo);
    glBindBuffer(GL_ARRAY_BUFFER, cbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    return 0;
}


void Engine::render_loop(){
    bool running = true;
    GLuint program = Shader::createProgram(vertexShaderSource, fragmentShaderSource);
    GLint posLoc = glGetAttribLocation(program, "vPosition");
    GLint colorLoc = glGetAttribLocation(program, "vColor");
    GLint mvpLoc = glGetUniformLocation(program, "u_mvpMatrix");

    glUseProgram(program);
    
    while(running){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT){
                running = false;
            }
        }

        glClearColor(0.2f, 0.4f, 0.6f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        mCubeRotation += 1.0f; // Increment rotation angle
        if (mCubeRotation > 360.0f) mCubeRotation -= 360.0f;

        // --- MATRIX CALCULATIONS ---
        float projMatrix[16], viewMatrix[16], modelMatrix[16];
        float rotXMatrix[16], rotYMatrix[16];
        float mvpMatrix[16], tempMatrix[16];

        matrixPerspective(projMatrix, 60.0f, 800.0f/600.0f, 1.0f, 20.0f);
        matrixTranslate(viewMatrix, 0.0f, 0.0f, -5.0f);

        matrixRotateX(rotXMatrix, mCubeRotation);
        matrixRotateY(rotYMatrix, mCubeRotation);
        matrixMultiply(modelMatrix, rotYMatrix, rotXMatrix); // Combine rotations

        // Create MVP matrix: Projection * View * Model
        matrixMultiply(tempMatrix, viewMatrix, modelMatrix);
        matrixMultiply(mvpMatrix, projMatrix, tempMatrix);

        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvpMatrix);

        // --- RENDER ---
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* // Position attribute
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(posLoc);

        // Color attribute
        glBindBuffer(GL_ARRAY_BUFFER, cbo);
        glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(colorLoc);
        
        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); */

        glUniformMatrix4fv(glGetUniformLocation(program, "u_mvpMatrix"), 1, GL_FALSE, mvpMatrix);

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
