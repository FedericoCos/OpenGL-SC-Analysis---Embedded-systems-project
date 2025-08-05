#version 330 core
// Sample from compTex and output to the default framebuffer
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D tex;
void main(){
    FragColor = texture(tex, vUV);
}
