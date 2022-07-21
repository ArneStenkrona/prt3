precision mediump float;

uniform float u_NearPlane;
uniform float u_FarPlane;

uniform sampler2D u_RenderTexture;
uniform sampler2D u_DepthBuffer;

varying vec2 v_TexCoordinate;

void main() {
    float z = texture2D(u_DepthBuffer, v_TexCoordinate).r;
    float n = u_NearPlane;
    float f = u_FarPlane;
    float c = (2.0 * n) / (f + n - z * (f - n));

    gl_FragColor = vec4(vec3(c), 1.0);
}
