#version 100

precision mediump float;

varying vec4 Color;

void main()
{
    gl_FragColor = vec4(Color.xyz, 1.0);
}
