#define STB_IMAGE_IMPLEMENTATION
#include "main.h"

int Engine::init(){
    // Initialize SDL and create a window
    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    window = SDL_CreateWindow("COMPUTE SHADER ES", 0, 0, WIN_WIDTH, WIN_HEIGHT,
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

    tracker.init();

    init_textures();

    //Simulated compute pipeline initialization
    init_compute();

    eglSwapInterval(eglDisplay, 0);

    return 0;
}

void Engine::init_textures(){
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char *data = stbi_load("textures/wall.jpg", &width, &height, &nrChannels, 0);
    if(!data){
        std::cout << "Failed to load texture" << std::endl;
        exit(0);
    }
    wallW = width; wallH = height; wallBpp = (nrChannels == 4 ? 4 : 3);

    glGenTextures(1, &wallTex);
    glBindTexture(GL_TEXTURE_2D, wallTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    tracker.countTextureBind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    tracker.trackVramAllocation(size_t(width) * size_t(height) * 3);
    tracker.trackDataUpload(size_t(width) * size_t(height) * 3);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
}

void Engine::render_loop(){
    running = true;

    while(running){
        tracker.beginFrame();

        float temp = (float)SDL_GetTicks() / 1000.f;
        dtime = temp - last;
        last = temp;

        process_input();

        float t = (float)SDL_GetTicks() / 1000.f;
        tracker.beginCpuRender();
        compute_pass(t);
        tracker.endCpuRender();

        eglSwapBuffers(eglDisplay, eglSurface);

        tracker.endFrame();
        tracker.printStats();
    }
    
    // Cleanup
    // Delete programs
    if (computeProg) { glDeleteProgram(computeProg); computeProg = 0; }
    if (blitProg)    { glDeleteProgram(blitProg);    blitProg    = 0; }

    // Delete fullscreen quad VBO
    if (quadVBO) {
        glDeleteBuffers(1, &quadVBO);
        quadVBO = 0;
        // 6 triangles verts * 2 components (x,y) * sizeof(GLfloat)
        tracker.trackVramDeallocation(size_t(6) * size_t(2) * sizeof(GLfloat));
    }

    // Delete compute output texture + FBO
    if (compFBO) { glDeleteFramebuffers(1, &compFBO); compFBO = 0; }
    if (compTex) {
        glDeleteTextures(1, &compTex);
        compTex = 0;
        // RGBA8 (4 bytes per pixel) * framebuffer size
        tracker.trackVramDeallocation(size_t(WIN_WIDTH) * size_t(WIN_HEIGHT) * 4);
    }

    // Delete wall texture (if created)
    if ( wallTex) {
        glDeleteTextures(1, & wallTex);
         wallTex = 0;
        tracker.trackVramDeallocation(size_t(wallW) * size_t(wallH) * 3);
    }

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

}

// Helper functions to load & compile GLSL
static std::string readFile(const std::string& path) {
    std::ifstream in(path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static GLuint compileShader(GLenum type, const std::string& src) {
    GLuint shader = glCreateShader(type);
    const char* cstr = src.c_str();
    glShaderSource(shader, 1, &cstr, nullptr);
    glCompileShader(shader);

    // Early exit on error
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetShaderInfoLog(shader, len, nullptr, &log[0]);
        std::cerr << "Shader compile error:\n" << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint linkProgram(GLuint vs, GLuint fs) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetProgramInfoLog(prog, len, nullptr, &log[0]);
        std::cerr << "Program link error:\n" << log << std::endl;
        glDeleteProgram(prog);
        return 0;
    }
    return prog;
}


// Engine::init_compute
void Engine::init_compute() {
    // 1) Load & compile shaders
    std::string vsrc = readFile("shaders/compute.vert");
    std::string fsrc = readFile("shaders/compute.frag");
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsrc);

    computeProg = linkProgram(vs, fs);
    computePosLoc = glGetAttribLocation(computeProg, "aPos");

    const char* blitV = R"(
        attribute vec2 aPos;
        varying vec2 uv;
        void main(){ uv=aPos*0.5+0.5; gl_Position=vec4(aPos,0,1); }
    )";
    const char* blitF = R"(
        precision mediump float;
        varying vec2 uv;
        uniform sampler2D tex;
        void main(){ gl_FragColor=texture2D(tex, uv); }
    )";
    GLuint vs2 = compileShader(GL_VERTEX_SHADER,   blitV);
    GLuint fs2 = compileShader(GL_FRAGMENT_SHADER, blitF);
    blitProg = linkProgram(vs2, fs2);
    blitPosLoc = glGetAttribLocation(blitProg, "aPos");


    // 2) Fullscreen quad setup
    GLfloat quad[] = {
        -1.f,-1.f,   +1.f,-1.f,  -1.f,+1.f,
        +1.f,-1.f,   +1.f,+1.f,  -1.f,+1.f
    };
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    tracker.trackVramAllocation(sizeof(quad));
    tracker.trackDataUpload(sizeof(quad));

    // 3) Create float‐texture & FBO
    glGenTextures(1, &compTex);
    glBindTexture(GL_TEXTURE_2D, compTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIN_WIDTH, WIN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    tracker.trackVramAllocation(size_t(WIN_WIDTH) * size_t(WIN_HEIGHT) * 4);

    glGenFramebuffers(1, &compFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, compFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, compTex, 0); 
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "compFBO incomplete\n"; exit(1);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

// Engine::compute_pass
void Engine::compute_pass(float time) {
    // Bind FBO & run “compute” shader to texture
    glBindFramebuffer(GL_FRAMEBUFFER, compFBO);
    glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(computeProg);
    tracker.countShaderBind();

    // Set uniforms
    glUniform1f(glGetUniformLocation(computeProg, "uTime"), time);
    glUniform2f(glGetUniformLocation(computeProg, "uResolution"),
                (float)WIN_WIDTH, (float)WIN_HEIGHT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,  wallTex);
    tracker.countTextureBind();
    GLint ch0 = glGetUniformLocation(computeProg, "iChannel0");
    if ( ch0 >= 0 ) glUniform1i(ch0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glEnableVertexAttribArray(computePosLoc);
    glVertexAttribPointer(computePosLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    tracker.countDrawCall();
    tracker.countTriangles(2);
    glDisableVertexAttribArray(computePosLoc);

    // Simple fullscreen‐blit into default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(blitProg);
    tracker.countShaderBind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, compTex);
    tracker.countTextureBind();
    glUniform1i(glGetUniformLocation(blitProg,"tex"), 0);

    // draw quad again:
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glEnableVertexAttribArray(blitPosLoc);
    glVertexAttribPointer(blitPosLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES,0,6);
    tracker.countDrawCall();
    tracker.countTriangles(2);
    glDisableVertexAttribArray(blitPosLoc);

}


int main(int argc, char * argv[]){
    Engine engine;

    if(engine.init()){
        return -1;
    };

    engine.render_loop();


    return 0;
}
