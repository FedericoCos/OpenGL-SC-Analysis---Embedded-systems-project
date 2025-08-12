#version 100
precision mediump float;

// Vertex attributes (inputs)
attribute vec3 aPos;
attribute vec3 aNormal;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;
uniform vec4 color;

varying vec4 Color;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    Color = color;
}
