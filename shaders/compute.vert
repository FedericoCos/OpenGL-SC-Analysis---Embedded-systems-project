// GLSL ES 1.00
precision mediump float;
attribute vec2 aPos;          // formerly "layout(location=0) in vec2 aPos;"
varying   vec2 vFragCoord;    // pass to fragment

void main() {
    // aPos ∈ [-1,1] → vFragCoord ∈ [0,1]
    vFragCoord = aPos * 0.5 + 0.5;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
