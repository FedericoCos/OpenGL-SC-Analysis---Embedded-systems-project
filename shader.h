#pragma once

#include <GLES2/gl2.h>
#include <iostream>

class Shader{
public:
    static GLuint loadShader(GLenum type, const char* shaderSrc) {
        GLuint shader = glCreateShader(type);
        if (shader == 0) return 0;
        glShaderSource(shader, 1, &shaderSrc, NULL);
        glCompileShader(shader);
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    static GLuint createProgram(const char* vShader, const char* fShader) {
        GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vShader);
        GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fShader);
        GLuint program = glCreateProgram();
        if (program == 0) return 0;
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        GLint linked;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (!linked) {
            glDeleteProgram(program);
            return 0;
        }
        return program;
    }
};