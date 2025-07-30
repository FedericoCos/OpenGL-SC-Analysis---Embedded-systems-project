// GLES
#include <GLES2/gl2.h>
#include <EGL/egl.h>

// EGL
#include <EGL/eglext.h>

// SDL2
#include <SDL2/SDL.h> // For Events
#include <SDL2/SDL_syswm.h>


#include <iostream>
#include <cmath>

#include "shader.h"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

class Engine{
public:

    int init();
    void render_loop();

private:
    SDL_Window* window;
    bool fullscreen = false;

    EGLDisplay eglDisplay;
    EGLSurface eglSurface;
    EGLContext eglContext;

    float mCubeRotation = 0.0f;

    float vertices[24] = {
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f
    };

    float colors[32] = {
        0.0f,  1.0f,  0.0f,  1.0f,
        0.0f,  1.0f,  0.0f,  1.0f,
        1.0f,  0.5f,  0.0f,  1.0f,
        1.0f,  0.5f,  0.0f,  1.0f,
        1.0f,  0.0f,  0.0f,  1.0f,
        1.0f,  0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,  1.0f,
        1.0f,  0.0f,  1.0f,  1.0f
    };

    GLubyte indices[36] = {
        0, 4, 5, 0, 5, 1,
        1, 5, 6, 1, 6, 2,
        2, 6, 7, 2, 7, 3,
        3, 7, 4, 3, 4, 0,
        4, 7, 6, 4, 6, 5,
        3, 0, 1, 3, 1, 2
    };



    // TO MOVE AWAY OR DELETE
    void matrixIdentity(float* m) {
        for(int i=0; i<16; i++) m[i] = (i%5==0) ? 1.0f : 0.0f;
    }

    void matrixMultiply(float* dst, const float* a, const float* b) {
        float temp[16];
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                temp[i * 4 + j] = a[i * 4 + 0] * b[0 * 4 + j] +
                                a[i * 4 + 1] * b[1 * 4 + j] +
                                a[i * 4 + 2] * b[2 * 4 + j] +
                                a[i * 4 + 3] * b[3 * 4 + j];
            }
        }
        for(int i=0; i<16; i++) dst[i] = temp[i];
    }

    void matrixRotateX(float* m, float angle) {
        float rad = angle * M_PI / 180.0f;
        float s = sin(rad), c = cos(rad);
        matrixIdentity(m);
        m[5] = c; m[6] = s;
        m[9] = -s; m[10] = c;
    }

    void matrixRotateY(float* m, float angle) {
        float rad = angle * M_PI / 180.0f;
        float s = sin(rad), c = cos(rad);
        matrixIdentity(m);
        m[0] = c; m[2] = -s;
        m[8] = s; m[10] = c;
    }

    void matrixTranslate(float* m, float x, float y, float z) {
        matrixIdentity(m);
        m[12] = x; m[13] = y; m[14] = z;
    }

    void matrixPerspective(float* m, float fovY, float aspect, float zNear, float zFar) {
        matrixIdentity(m);
        float f = 1.0f / tan(fovY * (M_PI / 360.0f));
        m[0] = f / aspect;
        m[5] = f;
        m[10] = (zFar + zNear) / (zNear - zFar);
        m[11] = -1.0f;
        m[14] = (2.0f * zFar * zNear) / (zNear - zFar);
        m[15] = 0.0f;
    }

    const char* vertexShaderSource = R"(
        layout (location = 0) in vec3 vPosition
        layout (location = 1) in vec3 vColor;
        uniform mat4 u_mvpMatrix;
        out vec3 ourColor;


        void main() {
            gl_Position = u_mvpMatrix * vPosition;
            ourColor = vColor;
        }
    )";

    const char* fragmentShaderSource = R"(
        precision mediump float;
        out vec4 FragColor;  
        in vec3 ourColor;

        void main() {
            gl_FragColor = fragColor;
        }
    )";

    GLuint vbo, cbo, ibo;

};