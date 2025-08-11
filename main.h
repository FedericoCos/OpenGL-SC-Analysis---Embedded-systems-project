// GLES
#include <GLES2/gl2.h>
#include <EGL/egl.h>

// EGL
#include <EGL/eglext.h>

#include "stb_image.h"

// SDL2
#include <SDL2/SDL.h> // For Events
#include <SDL2/SDL_syswm.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <cmath>

#include "perfTracker.h"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

class Engine{
public:

    int init();
    void render_loop();

private:
    // Window / platform
    SDL_Window* window = nullptr;
    SDL_Event   event{};
    bool fullscreen = false;
    bool running    = false;

    // Time  
    float dtime = 0.f;
    float last  = 0.f;

    // EGL  
    EGLDisplay eglDisplay = EGL_NO_DISPLAY;
    EGLSurface eglSurface = EGL_NO_SURFACE;
    EGLContext eglContext = EGL_NO_CONTEXT;

    // Performance tracker  
    PerfTracker tracker;

    // Input texture (wall.jpg)  
    GLuint wallTex = 0;
    int wallW = 0, wallH = 0, wallBpp = 0; // bytes per pixel (3 for RGB, 4 for RGBA)

    // “Compute” pipeline (fragment-based)  
    GLuint computeProg = 0;   // program for compute-as-fragment
    GLuint blitProg    = 0;   // program for blit
    GLuint quadVBO     = 0;   // fullscreen quad VBO
    GLuint compTex     = 0;   // compute output texture (RGBA8)
    GLuint compFBO     = 0;   // FBO for compute output
    GLint  computePosLoc = -1;
    GLint  blitPosLoc    = -1;

    // Helpers  
    void process_input();
    void init_textures();       // loads wallTex
    void init_compute();        // sets up compute/blit pipeline
    void compute_pass(float t); // runs compute then blit
};