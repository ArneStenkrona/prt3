precision mediump float;

uniform float u_NearPlane;
uniform float u_FarPlane;

uniform float u_PixelUnitX;
uniform float u_PixelUnitY;

varying vec2 v_TexCoordinate;

uniform sampler2D u_RenderTexture;

void main() {
    gl_FragColor = texture2D(u_RenderTexture, v_TexCoordinate + 0.005*vec2( sin(1024.0*v_TexCoordinate.x),cos(768.0*v_TexCoordinate.y)));
}
