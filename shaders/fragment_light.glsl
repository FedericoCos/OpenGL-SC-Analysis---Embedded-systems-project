#version 330 core
out vec4 FragColor;

in vec4 Color;

void main(){
    FragColor = vec4(Color.xyz, 1.0);
}