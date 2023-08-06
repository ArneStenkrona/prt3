#version 300 es

precision mediump float;

uniform sampler2D u_DepthMap;
uniform sampler2D u_DecalMap;

uniform mat4 u_InvMMatrix;
uniform mat4 u_InvVP;

uniform int u_BufferWidth;
uniform int u_BufferHeight;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 screenUV = gl_FragCoord.xy /
                    vec2(float(u_BufferWidth), float(u_BufferHeight));
    ivec2 texelPos = ivec2(gl_FragCoord.xy);

    float depth = texelFetch(u_DepthMap, texelPos, 0).r;
    // float depth = texture(u_DepthMap, screenUV).r;

    vec4 p = u_InvVP * (vec4(screenUV, depth, 1.0) * 2.0 - 1.0);
    vec4 worldPos = p / p.w;

    vec4 mpos = u_InvMMatrix * worldPos;
    if (any(greaterThan(abs(mpos.xyz), vec3(0.5)))) discard;

    vec2 decalUV = mpos.xz + 0.5;
    vec4 decalColor = texture(u_DecalMap, decalUV);
    outColor = decalColor;
}
