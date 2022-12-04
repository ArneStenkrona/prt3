#version 300 es

precision mediump float;

in vec3 v_Position;

layout(location = 0) out vec3 outColor;

void main() {
    outColor = vec3(0.0, 1.0, 0.0);
}
