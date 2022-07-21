precision mediump float;

varying vec2 v_TexCoordinate;

uniform sampler2D u_RenderTexture;

void main() {
    float time = 0.0;
    gl_FragColor = texture2D( u_RenderTexture, v_TexCoordinate);
}
