precision mediump float;

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
    return (2.0 * n) / (f + n - z * (f - n));
}

void main() {
    float z = texture2D(u_DepthBuffer, v_TexCoordinate).r;

    float w = u_PixelUnitX;
    float h = u_PixelUnitY;

    float n0 = linearize_depth(texture2D(u_DepthBuffer, v_TexCoordinate + vec2( -w, -h)).r);
    // float n1 = linearize_depth(texture2D(u_DepthBuffer, v_TexCoordinate + vec2(0.0, -h)).r);
    float n2 = linearize_depth(texture2D(u_DepthBuffer, v_TexCoordinate + vec2(  w, -h)).r);
    // float n3 = linearize_depth(texture2D(u_DepthBuffer, v_TexCoordinate + vec2( -w, 0.0)).r);
    float n4 = linearize_depth(texture2D(u_DepthBuffer, v_TexCoordinate).r);
    // float n5 = linearize_depth(texture2D(u_DepthBuffer, v_TexCoordinate + vec2(  w, 0.0)).r);
    float n6 = linearize_depth(texture2D(u_DepthBuffer, v_TexCoordinate + vec2( -w, h)).r);
    // float n7 = linearize_depth(texture2D(u_DepthBuffer, v_TexCoordinate + vec2(0.0, h)).r);
    float n8 = linearize_depth(texture2D(u_DepthBuffer, v_TexCoordinate + vec2(  w, h)).r);

    // float sobel_edge_h = n2 + (2.0*n5) + n8 - (n0 + (2.0*n3) + n6);
    // float sobel_edge_v = n0 + (2.0*n1) + n2 - (n6 + (2.0*n7) + n8);
    // float sobel = sqrt((sobel_edge_h * sobel_edge_h) + (sobel_edge_v * sobel_edge_v));
    float eps = 0.1;

    float horizontal = n0 - n8;
    if (min(n0, n8) + eps < n4) {
        horizontal = 0.0;
    }
    float vertical = n2 - n6;
    if (min(n2, n6) + eps < n4) {
        vertical = 0.0;
    }
    float robert_cross = sqrt(dot(horizontal, horizontal) + dot(vertical, vertical));
    vec3 outline_color = vec3(1.0);
    vec3 render_color = texture2D(u_RenderTexture, v_TexCoordinate).rgb;
    float threshold = 0.01;
    vec3 color = robert_cross > threshold ? outline_color : render_color;

    gl_FragColor = vec4(color, 1.0);
}
