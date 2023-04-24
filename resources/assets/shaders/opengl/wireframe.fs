#version 300 es

precision mediump float;

uniform vec4 u_Color;

in vec3 v_Position;

layout(location = 0) out vec3 outColor;

void main() {
    outColor = u_Color.rgb;
}
