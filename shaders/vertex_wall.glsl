#version 100
// Note: GLSL ES 1.00 does not use version 'core'

// 'in' becomes 'attribute' for vertex inputs
attribute vec3 aPos;
attribute vec3 aNormal;
attribute vec3 aColor;

// Uniforms remain the same
uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

// 'out' becomes 'varying' to pass data to the fragment shader
varying vec3 Normal;
varying vec3 FragPos;
varying vec3 Color;
varying vec4 FragPosLightSpace;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    // The calculation for Normal is fine, but ensure your matrices are invertible
    Normal = mat3(model) * aNormal; // Simplified normal transformation, often sufficient

    Color = aColor;

    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
}
