#version 300 es

uniform mat4 u_MMatrix;
uniform mat4 u_MVPMatrix;
uniform mat4 u_MVMatrix;
uniform mat3 u_InvTposMMatrix;

uniform float u_NearPlane;
uniform float u_FarPlane;

layout(location = 0) in vec3 a_Position;

out vec3 v_Position;

// The entry point for our vertex shader.
void main()
{
    v_Position = vec3(u_MMatrix * vec4(a_Position, 1.0));

    gl_Position = (u_MVPMatrix * vec4(a_Position, 1.0));
}
