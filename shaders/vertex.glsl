layout (location = 0) in vec3 vPosition
layout (location = 1) in vec3 vColor;
uniform mat4 u_mvpMatrix;
out vec3 ourColor;


void main() {
    gl_Position = u_mvpMatrix * vPosition;
    ourColor = vColor;
}