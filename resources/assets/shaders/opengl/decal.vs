#version 300 es

uniform mat4 u_MVPMatrix;

uniform float u_NearPlane;
uniform float u_FarPlane;

layout(location = 0) in vec3 a_Position;

void main() {
    gl_Position = u_MVPMatrix * vec4(a_Position, 1.0);
}
