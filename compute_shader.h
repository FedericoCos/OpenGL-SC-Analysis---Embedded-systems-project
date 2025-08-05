// compute_shader.h
#pragma once
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// Utility to compile & link a single .comp file into a compute program.
class ComputeShader {
public:
    GLuint ID;
    ComputeShader(const char* path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "ERROR: could not open compute shader '" << path << "'" << std::endl;
            return;
        }
        std::stringstream ss; ss << file.rdbuf();
        std::string src = ss.str();
        const char* cstr = src.c_str();

        GLuint comp = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(comp, 1, &cstr, nullptr);
        glCompileShader(comp);
        GLint ok = 0; glGetShaderiv(comp, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[1024]; glGetShaderInfoLog(comp, 1024, nullptr, log);
            std::cerr << "COMPUTE SHADER COMPILE ERROR:\n" << log << std::endl;
        }

        ID = glCreateProgram();
        glAttachShader(ID, comp);
        glLinkProgram(ID);
        glDeleteShader(comp);
        glGetProgramiv(ID, GL_LINK_STATUS, &ok);
        if (!ok) {
            char log[1024]; glGetProgramInfoLog(ID, 1024, nullptr, log);
            std::cerr << "COMPUTE PROGRAM LINK ERROR:\n" << log << std::endl;
        }
    }

    void use()     { glUseProgram(ID); }
    void dispatch(GLuint x, GLuint y, GLuint z) { glDispatchCompute(x,y,z); }
    void barrier() { glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); }
};