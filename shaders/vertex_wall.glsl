#version 100
precision mediump float;

attribute vec3 aPos;
attribute vec3 aNormal;
attribute vec3 aColor;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

varying vec3 Normal;
varying vec3 FragPos;
varying vec3 Color;
varying vec4 FragPosLightSpace;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(model) * aNormal; // Simplified normal transformation, often sufficient

    Color = aColor;

    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
}
