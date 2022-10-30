#version 300 es

precision mediump float;

in vec3 v_Position;
in vec3 v_Normal;
in vec2 v_TexCoordinate;
in mat3 v_InverseTBN;

layout(location = 0) out float outSelected;

void main() {
    outSelected = 1.0;
}
