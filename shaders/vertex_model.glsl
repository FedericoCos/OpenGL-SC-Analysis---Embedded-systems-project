#version 100
// GLSL for OpenGL ES 2.0

// Vertex attributes
attribute vec3 aPos;
attribute vec3 aNormal;
attribute vec2 aTexCoords;

// Uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

// Varyings (outputs to fragment shader)
varying vec2 TexCoords;
varying vec4 FragPosLightSpace;
varying vec3 Normal;
varying vec3 FragPos;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    Normal = mat3(model) * aNormal;
    
    TexCoords = aTexCoords;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
}