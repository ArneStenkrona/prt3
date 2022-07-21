precision mediump float;

uniform float u_NearPlane;
uniform float u_FarPlane;

varying vec2 v_TexCoordinate;

uniform sampler2D u_RenderTexture;

void main() {
    gl_FragColor = texture2D( u_RenderTexture, v_TexCoordinate);
}
