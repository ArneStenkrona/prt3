#version 300 es

precision mediump float;

in vec2 v_UV;
in vec4 v_Color;

uniform sampler2D u_Texture;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = v_Color * texture(u_Texture, v_UV);
}
