#pragma once

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

#ifndef GL_OES_get_program_binary
#define GL_OES_get_program_binary 1
typedef void (GL_APIENTRYP PFNGLGETPROGRAMBINARYOESPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
typedef void (GL_APIENTRYP PFNGLPROGRAMBINARYOESPROC) (GLuint program, GLenum binaryFormat, const void *binary, GLint length);
#endif

// Global function pointers (or put them in your Shader class if preferred)
PFNGLGETPROGRAMBINARYOESPROC glGetProgramBinaryOES_ptr = nullptr;
PFNGLPROGRAMBINARYOESPROC glProgramBinaryOES_ptr = nullptr;

class Shader {
public:
    // This function should be called once at application start to initialize extension pointers
    static void initGLExtensions() {
        const GLubyte* extensions = glGetString(GL_EXTENSIONS);
        if (extensions) {
            std::string extensions_str = reinterpret_cast<const char*>(extensions);
            std::cout << "OpenGL ES Extensions: " << extensions_str << std::endl;

            if (extensions_str.find("GL_OES_get_program_binary") != std::string::npos) {
                std::cout << "GL_OES_get_program_binary extension supported!" << std::endl;
                // Get function pointers. This part is platform-specific.
                // Example for EGL-based systems (like Android/Linux)
#ifdef EGL_VERSION_1_0
                glGetProgramBinaryOES_ptr = reinterpret_cast<PFNGLGETPROGRAMBINARYOESPROC>(eglGetProcAddress("glGetProgramBinaryOES"));
                glProgramBinaryOES_ptr = reinterpret_cast<PFNGLPROGRAMBINARYOESPROC>(eglGetProcAddress("glProgramBinaryOES"));

                if (!glGetProgramBinaryOES_ptr || !glProgramBinaryOES_ptr) {
                    std::cerr << "Warning: GL_OES_get_program_binary functions could not be loaded via eglGetProcAddress.\n";
                    glGetProgramBinaryOES_ptr = nullptr; // Ensure they are null if loading fails
                    glProgramBinaryOES_ptr = nullptr;
                }
#endif
                // For iOS/Apple, these are usually just available if headers are included.
                // No explicit eglGetProcAddress needed.
            } else {
                std::cerr << "Warning: GL_OES_get_program_binary extension NOT supported. Cannot save/load program binaries.\n";
            }
        } else {
            std::cerr << "Error: glGetString(GL_EXTENSIONS) returned null.\n";
        }
    }

    static GLuint loadShader(GLenum type, const char* shaderSrc) {
        GLuint shader = glCreateShader(type);
        if (shader == 0) {
            std::cerr << "Error: Failed to create shader of type " << type << "\n";
            return 0;
        }
        glShaderSource(shader, 1, &shaderSrc, NULL);
        glCompileShader(shader);
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen > 1) { // Greater than 1 because 1 is just the null terminator
                std::vector<char> infoLog(infoLen);
                glGetShaderInfoLog(shader, infoLen, NULL, infoLog.data());
                std::cerr << "Error compiling shader type " << type << ":\n" << infoLog.data() << "\n";
            }
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    static GLuint createProgram(const char* vShaderSrc, const char* fShaderSrc) {
        GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vShaderSrc);
        if (vertexShader == 0) {
            std::cerr << "Failed to load vertex shader.\n";
            return 0;
        }
        GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fShaderSrc);
        if (fragmentShader == 0) {
            std::cerr << "Failed to load fragment shader.\n";
            glDeleteShader(vertexShader);
            return 0;
        }

        GLuint program = glCreateProgram();
        if (program == 0) {
            std::cerr << "Error: Failed to create program.\n";
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return 0;
        }
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        GLint linked;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (!linked) {
            GLint infoLen = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen > 1) {
                std::vector<char> infoLog(infoLen);
                glGetProgramInfoLog(program, infoLen, NULL, infoLog.data());
                std::cerr << "Error linking program:\n" << infoLog.data() << "\n";
            }
            glDeleteProgram(program);
            program = 0; // Indicate failure
        }

        // Detach and delete shaders after linking
        // They are no longer needed as they are linked into the program
        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }

    // Function to save the compiled program binary
    static bool saveProgramBinary(GLuint program, const char* filename) {
        if (!glGetProgramBinaryOES_ptr) {
            std::cerr << "Error: GL_OES_get_program_binary extension not available or functions not loaded. Cannot save program binary.\n";
            return false;
        }

        GLint binaryLength = 0;
        glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH_OES, &binaryLength); // Use _OES suffix for enum

        if (binaryLength == 0) {
            std::cerr << "Warning: Program binary length is 0. Nothing to save.\n";
            return false;
        }

        std::vector<GLubyte> binary(binaryLength);
        GLenum format = 0;
        GLsizei lengthWritten = 0; // Will hold the actual length written to 'binary'

        glGetProgramBinaryOES_ptr(program, binaryLength, &lengthWritten, &format, binary.data());

        if (lengthWritten != binaryLength) {
            std::cerr << "Error: Inconsistent binary length reported by glGetProgramBinaryOES.\n";
            return false;
        }

        std::ofstream out(filename, std::ios::binary);
        if (!out.is_open()) {
            std::cerr << "Error: Could not open " << filename << " for writing program binary.\n";
            return false;
        }

        out.write(reinterpret_cast<char*>(&format), sizeof(format));
        out.write(reinterpret_cast<char*>(binary.data()), binary.size());
        out.close();

        std::cout << "Program binary saved successfully to " << filename << " (Format: 0x" << std::hex << format << std::dec << ", Size: " << binary.size() << " bytes)\n";
        return true;
    }

    // Function to load the program from a binary file
    static GLuint loadProgramBinary(const char* filename) {
        if (!glProgramBinaryOES_ptr) {
            std::cerr << "Error: GL_OES_get_program_binary extension not available or functions not loaded. Cannot load program binary.\n";
            return 0;
        }

        std::ifstream in(filename, std::ios::binary);
        if (!in.is_open()) {
            std::cerr << "Failed to open program binary file: " << filename << "\n";
            return 0;
        }

        GLenum format;
        in.read(reinterpret_cast<char*>(&format), sizeof(format));

        // Get the remaining size of the binary data
        in.seekg(0, std::ios::end);
        GLsizei size = static_cast<GLsizei>(in.tellg()) - sizeof(format);
        in.seekg(sizeof(format), std::ios::beg); // Seek back to the start of the binary data

        std::vector<GLubyte> binary(size);
        in.read(reinterpret_cast<char*>(binary.data()), size);
        in.close();

        GLuint program = glCreateProgram();
        if (program == 0) {
            std::cerr << "Error: Failed to create program object for binary loading.\n";
            return 0;
        }

        glProgramBinaryOES_ptr(program, format, binary.data(), size);

        GLint linked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (!linked) {
            GLint infoLen = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen > 1) {
                std::vector<char> infoLog(infoLen);
                glGetProgramInfoLog(program, infoLen, NULL, infoLog.data());
                std::cerr << "Program binary load error for " << filename << ": " << infoLog.data() << "\n";
            } else {
                 std::cerr << "Program binary load failed for " << filename << " (no info log available).\n";
            }
            glDeleteProgram(program);
            return 0;
        }

        std::cout << "Program binary loaded successfully from " << filename << "\n";
        return program;
    }

    // Combined function to load or create and save the program
    static GLuint loadOrCreateProgram(const char* binaryFile, const char* vShaderSrc, const char* fShaderSrc) {
        GLuint program = 0;

        // 1. Try to load from binary first
        program = loadProgramBinary(binaryFile);
        if (program != 0) {
            return program; // Successfully loaded from binary
        }

        // 2. If binary load failed, compile from source
        std::cerr << "Falling back to compiling shaders from source...\n";
        program = createProgram(vShaderSrc, fShaderSrc);

        if (program != 0) {
            std::cout << "Successfully compiled and linked program from source.\n";
            // 3. If compiled successfully, try to save the new binary
            saveProgramBinary(program, binaryFile);
        } else {
            std::cerr << "Fatal Error: Could not load program from binary or compile from source!\n";
        }

        return program;
    }
};