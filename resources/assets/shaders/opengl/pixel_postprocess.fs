precision mediump float;

uniform vec3 u_ViewPosition;
uniform vec3 u_ViewDirection;
uniform mat4 u_InvVMatrix;
uniform mat4 u_InvPMatrix;

uniform float u_NearPlane;
uniform float u_FarPlane;

uniform float u_PixelUnitX;
uniform float u_PixelUnitY;

uniform sampler2D u_PreviousColorBuffer;
uniform sampler2D u_NormalBuffer;
uniform sampler2D u_DepthBuffer;

varying vec2 v_TexCoordinate;

float linearize_depth(float z) {
    float n = u_NearPlane;
    float f = u_FarPlane;
    // return (2.0 * near * far) / (far + near - z * (far - near));
    return (2.0 * n) / (f + n - z * (f - n));
}

void main() {
    float z = texture2D(u_DepthBuffer, v_TexCoordinate).r;
    vec4 ndc = vec4(2.0 * v_TexCoordinate.x - 1.0, 2.0 * v_TexCoordinate.y - 1.0, 2.0 * z - 1.0, 1.0);
    vec4 vs_pos = u_InvPMatrix * ndc;
    vs_pos = vs_pos / vs_pos.w;
    vec4 world_pos = u_InvVMatrix * vs_pos;

    float w = u_PixelUnitX;
    float h = u_PixelUnitY;

    float depth = linearize_depth(texture2D(u_DepthBuffer, v_TexCoordinate).r);
    // Difference between depth of neighboring pixels and current.
    float depth_diff = 0.0;
    float d_right = linearize_depth(texture2D(u_DepthBuffer, v_TexCoordinate + vec2( w, 0.0)).r);
    float d_left  = linearize_depth(texture2D(u_DepthBuffer, v_TexCoordinate + vec2(-w, 0.0)).r);
    float d_down  = linearize_depth(texture2D(u_DepthBuffer, v_TexCoordinate + vec2(0.0,  h)).r);
    float d_up    = linearize_depth(texture2D(u_DepthBuffer, v_TexCoordinate + vec2(0.0, -h)).r);
    float eps = 0.0;
    if (d_right < u_FarPlane - eps) {
        depth_diff += -min(depth - d_right, 0.0);
    }
    if (d_left < u_FarPlane - eps) {
        depth_diff += -min(depth - d_left, 0.0);
    }
    if (d_down < u_FarPlane - eps) {
        depth_diff += -min(depth - d_down, 0.0);
    }
    if (d_up < u_FarPlane - eps) {
        depth_diff += -min(depth - d_up, 0.0);
    }

    vec3 normal = 2.0 * texture2D(u_NormalBuffer, v_TexCoordinate).rgb - 1.0;
    // Difference between depth of neighboring pixels and current.
    float normal_diff = 0.0;
    vec3 n_right = 2.0 * texture2D(u_NormalBuffer, v_TexCoordinate + vec2( w,  0.0)).rgb - 1.0;
    vec3 n_left  = 2.0 * texture2D(u_NormalBuffer, v_TexCoordinate + vec2(-w,  0.0)).rgb - 1.0;
    vec3 n_down  = 2.0 * texture2D(u_NormalBuffer, v_TexCoordinate + vec2(0.0,  h)).rgb - 1.0;
    vec3 n_up    = 2.0 * texture2D(u_NormalBuffer, v_TexCoordinate + vec2(0.0, -h)).rgb - 1.0;
    if (n_right.z > normal.z) {
        normal_diff += distance(normal, n_right);
    }
    if (n_left.z > normal.z) {
        normal_diff += distance(normal, n_left);
    }
    if (n_down.z > normal.z) {
        normal_diff += distance(normal, n_down);
    }
    if (n_up.z > normal.z) {
        normal_diff += distance(normal, n_up);
    }
    float normal_outline = normal_diff > 0.1 ? 1.0 : 0.0;

    vec3 V = normalize(u_ViewPosition - world_pos.xyz);
    float n_v_angle = acos(dot(normal, V));
    float grazing = pow(max(1.0 - max(dot(normal, V), 0.0), 0.0), 10.0);
    float depth_outline = depth_diff > mix(0.001, 0.02, grazing) ? 1.0 : 0.0;

    vec3 color;
    //if (normal_outline > depth_outline) {
    //    color = 0.5 * texture2D(u_PreviousColorBuffer, v_TexCoordinate).rgb;
    //} else if (depth_outline > 0.0) {
    //    color = 0.3 * texture2D(u_PreviousColorBuffer, v_TexCoordinate).rgb;
    // } else {
        color = texture2D(u_PreviousColorBuffer, v_TexCoordinate).rgb;
    // }

    float x = gl_FragCoord.x;
    x = x - (4.0 * floor(x/4.0));
    float y = gl_FragCoord.y;
    y = y - (4.0 * floor(y/4.0));

    int index = int(x + y * 4.0);
    float M = 0.0;

    if (index == 0) M = 0.0625;
    if (index == 1) M = 0.5625;
    if (index == 2) M = 0.1875;
    if (index == 3) M = 0.6875;
    if (index == 4) M = 0.8125;
    if (index == 5) M = 0.3125;
    if (index == 6) M = 0.9375;
    if (index == 7) M = 0.4375;
    if (index == 8) M = 0.25;
    if (index == 9) M = 0.75;
    if (index == 10) M = 0.125;
    if (index == 11) M = 0.625;
    if (index == 12) M = 1.0;
    if (index == 13) M = 0.5;
    if (index == 14) M = 0.875;
    if (index == 15) M = 0.375;

    float noise = M;
    float spread = 0.05;

    vec3 dithered_color = color + spread * M;

    float n = 16.0;
    vec3 compressed_color = floor(dithered_color * (n - 1.0) + 0.5) / (n - 1.0);
    gl_FragColor = vec4(compressed_color, 1.0);

    // gl_FragColor = vec4(color, 1.0);
}
