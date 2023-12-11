#version 300 es

uniform mat4 u_VPMatrix;
uniform mat3 u_invVRotMatrix;

uniform vec2 u_InvDiv;

layout(location = 0) in vec3 a_VertexPos;
layout(location = 1) in vec4 a_PosSize;
layout(location = 2) in vec2 a_BaseUV;
layout(location = 3) in vec4 a_Color;

out vec3 v_Position;
out vec2 v_UV;
out vec4 v_Color;
out vec2 v_screenUV;

void main() {
    v_UV = a_BaseUV - u_InvDiv * a_VertexPos.xy;
    vec3 offset = (u_invVRotMatrix * a_PosSize.w * a_VertexPos.xyz);
    vec3 pos = a_PosSize.xyz + offset;
    v_Position = pos;

    v_Color = a_Color;

    gl_Position = u_VPMatrix * vec4(pos, 1.0);
    v_screenUV = (gl_Position.xy / gl_Position.w) * (0.5) + vec2(0.5);
}
