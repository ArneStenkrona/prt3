#version 300 es

precision mediump float;

uniform sampler2D u_DepthMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_DecalMap;

uniform mat4 u_InvMMatrix;
uniform mat4 u_InvVP;

uniform int u_BufferWidth;
uniform int u_BufferHeight;

uniform vec4 u_Color;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 screenUV = gl_FragCoord.xy /
                    vec2(float(u_BufferWidth), float(u_BufferHeight));
    ivec2 texelPos = ivec2(gl_FragCoord.xy);

    float depth = texelFetch(u_DepthMap, texelPos, 0).r;

    vec4 p = u_InvVP * (vec4(screenUV, depth, 1.0) * 2.0 - 1.0);
    vec4 worldPos = p / p.w;

    vec4 mpos = u_InvMMatrix * worldPos;
    if (any(greaterThan(abs(mpos.xyz), vec3(0.5)))) discard;

    vec3 mup = normalize(transpose(mat3(u_InvMMatrix)) * vec3(0.0, 1.0, 0.0));
    vec3 normal = 2.0 * texelFetch(u_NormalMap, texelPos, 0).xyz - 1.0;
    if (dot(normal, mup) < 0.70710678118) discard;

    vec2 decalUV = mpos.xz + 0.5;
    vec4 decalColor = u_Color * texture(u_DecalMap, decalUV);
    outColor = decalColor;
}
