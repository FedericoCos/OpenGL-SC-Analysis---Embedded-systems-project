#version 100
// GLSL for OpenGL ES 2.0

precision mediump float;

varying vec4 Color;

void main()
{
    // The final pixel color is written to the built-in gl_FragColor variable
    gl_FragColor = vec4(Color.xyz, 1.0);
}
