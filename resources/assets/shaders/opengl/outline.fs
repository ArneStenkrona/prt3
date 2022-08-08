precision mediump float;

uniform vec3 u_ViewPosition;
uniform vec3 u_ViewDirection;
uniform mat4 u_InvVMatrix;
uniform mat4 u_InvPMatrix;

uniform float u_NearPlane;
uniform float u_FarPlane;

uniform float u_PixelUnitX;
uniform float u_PixelUnitY;

uniform sampler2D u_RenderTexture;
uniform sampler2D u_NormalTexture;
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

    vec3 normal = 2.0 * texture2D(u_NormalTexture, v_TexCoordinate).rgb - 1.0;
    // Difference between depth of neighboring pixels and current.
    float normal_diff = 0.0;
    vec3 n_right = 2.0 * texture2D(u_NormalTexture, v_TexCoordinate + vec2( w,  0.0)).rgb - 1.0;
    vec3 n_left  = 2.0 * texture2D(u_NormalTexture, v_TexCoordinate + vec2(-w,  0.0)).rgb - 1.0;
    vec3 n_down  = 2.0 * texture2D(u_NormalTexture, v_TexCoordinate + vec2(0.0,  h)).rgb - 1.0;
    vec3 n_up    = 2.0 * texture2D(u_NormalTexture, v_TexCoordinate + vec2(0.0, -h)).rgb - 1.0;
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
    if (normal_outline > depth_outline) {
        color = 0.5 * texture2D(u_RenderTexture, v_TexCoordinate).rgb;
    } else if (depth_outline > 0.0) {
        color = 0.1 * texture2D(u_RenderTexture, v_TexCoordinate).rgb;
    } else {
        color = texture2D(u_RenderTexture, v_TexCoordinate).rgb;
    }

    float outline = max(depth_outline, normal_outline);
    // gl_FragColor = vec4(vec3(outline), 1.0);
    gl_FragColor = vec4(color, 1.0);
}
