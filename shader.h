#pragma once

// Use GLES2 headers instead of GLAD
#include <GLES2/gl2.h>

// Keep standard and GLM headers
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector> // Needed for robust error logging

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
    // The program ID, using the official GLuint type
    GLuint ID;

    // Default constructor
    Shader() : ID(0) {
        // Initialize ID to 0 to indicate an invalid/uninitialized shader
    }

    // Constructor reads, compiles, and links the shader program
    Shader(const char* vertexPath, const char* fragmentPath) {
        // 1. Retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        // ensure ifstream objects can throw exceptions
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure& e) {
            std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
            ID = 0; // Set ID to 0 to indicate failure
            return;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        // 2. Compile shaders
        GLuint vertex, fragment;
        GLint success;

        // Vertex Shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        // Check for compile errors
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLint infoLen = 0;
            glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen > 1) {
                std::vector<char> infoLog(infoLen);
                glGetShaderInfoLog(vertex, infoLen, NULL, infoLog.data());
                std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog.data() << std::endl;
            }
            glDeleteShader(vertex); // Don't leak the shader
            ID = 0;
            return;
        }

        // Fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        // Check for compile errors
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLint infoLen = 0;
            glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen > 1) {
                std::vector<char> infoLog(infoLen);
                glGetShaderInfoLog(fragment, infoLen, NULL, infoLog.data());
                std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog.data() << std::endl;
            }
            glDeleteShader(vertex); // Clean up both shaders on failure
            glDeleteShader(fragment);
            ID = 0;
            return;
        }

        // Shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        // Check for linking errors
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success) {
            GLint infoLen = 0;
            glGetProgramiv(ID, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen > 1) {
                std::vector<char> infoLog(infoLen);
                glGetProgramInfoLog(ID, infoLen, NULL, infoLog.data());
                std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog.data() << std::endl;
            }
            glDeleteProgram(ID); // Clean up the program
            ID = 0;
        }

        // Delete the shaders as they're linked into our program now and no longer necessary
        // Detaching is good practice before deleting
        glDetachShader(ID, vertex);
        glDetachShader(ID, fragment);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    // Activate the shader
    void use() const {
        if (ID != 0) {
            glUseProgram(ID);
        }
    }

    // Utility uniform functions
    void setBool(const std::string& name, bool value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void setInt(const std::string& name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setFloat(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setVec3(const std::string& name, const glm::vec3& vec) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(vec));
    }
    void setVec4(const std::string& name, const glm::vec4& vec) const {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(vec));
    }
    void setMatrix(const std::string& name, const glm::mat4& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }
};