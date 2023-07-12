#version 300 es

precision mediump float;

uniform vec3 u_ViewPosition;
uniform vec3 u_ViewDirection;
uniform mat4 u_InvVMatrix;
uniform mat4 u_InvPMatrix;

uniform float u_NearPlane;
uniform float u_FarPlane;

uniform float u_PixelUnitX;
uniform float u_PixelUnitY;

uniform uint u_Frame;

uniform sampler2D u_PreviousColorBuffer;
uniform sampler2D u_NormalBuffer;
uniform sampler2D u_NodeDataBuffer;
uniform sampler2D u_SelectedBuffer;
uniform sampler2D u_DepthBuffer;

in vec2 v_TexCoordinate;

layout(location = 0) out vec4 outColor;

float linearize_depth(float z) {
    float n = u_NearPlane;
    float f = u_FarPlane;
    return (2.0 * n) / (f + n - z * (f - n));
}

void main() {
    vec4 color = texture(u_PreviousColorBuffer, v_TexCoordinate);

    vec4 node_data = texture(u_NodeDataBuffer, v_TexCoordinate);
    bool selected_visible = node_data.a != 0.0;

    bool selected = texture(u_SelectedBuffer, v_TexCoordinate).r != 0.0;


    if (!selected_visible && selected) {
        color.rgb *= 0.25;
    }


    // float selectedFloat = texture(u_SelectedBuffer, v_TexCoordinate).r;
    // bool selected = selectedFloat == 1.0;

    // vec4 color = texture(u_PreviousColorBuffer, v_TexCoordinate);

    // float w = u_PixelUnitX;
    // float h = u_PixelUnitY;

    // float selected_diff = 0.0;
    // float s_right = texture(u_SelectedBuffer, v_TexCoordinate + vec2( w,  0.0)).r;
    // float s_left  = texture(u_SelectedBuffer, v_TexCoordinate + vec2(-w,  0.0)).r;
    // float s_down  = texture(u_SelectedBuffer, v_TexCoordinate + vec2(0.0,  h)).r;
    // float s_up    = texture(u_SelectedBuffer, v_TexCoordinate + vec2(0.0, -h)).r;

    // selected_diff += abs(selectedFloat - s_right);
    // selected_diff += abs(selectedFloat - s_left);
    // selected_diff += abs(selectedFloat - s_down);
    // selected_diff += abs(selectedFloat - s_up);

    // if (selected_diff > 0.0) {
    //     color.rgb = vec3(1.0, 0.65, 0.0);
    // }

    outColor = color;
}
