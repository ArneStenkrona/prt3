#version 300 es

uniform mat4 u_MMatrix;
uniform mat4 u_MVPMatrix;
uniform mat4 u_MVMatrix;
uniform mat3 u_InvTposMMatrix;

uniform float u_NearPlane;
uniform float u_FarPlane;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoordinate;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

out vec3 v_Position;
out vec3 v_Normal;
out vec2 v_TexCoordinate;
out mat3 v_InverseTBN;

void main() {
    v_Position = vec3(u_MMatrix * vec4(a_Position, 1.0));

    v_TexCoordinate = a_TexCoordinate;

    v_Normal = u_InvTposMMatrix * a_Normal;

    vec3 t = normalize(u_InvTposMMatrix * a_Tangent);
    vec3 b = normalize(u_InvTposMMatrix * a_Bitangent);
    vec3 n = normalize(u_InvTposMMatrix * a_Normal);

    v_InverseTBN = mat3(t,b,n);

    gl_Position = u_MVPMatrix * vec4(a_Position, 1.0);
}
