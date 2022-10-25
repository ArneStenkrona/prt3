precision mediump float;

uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_MetallicMap;
uniform sampler2D u_RoughnessMap;
uniform vec4 u_Albedo;
uniform float u_Metallic;
uniform float u_Roughness;

struct PointLight {
    vec3 position;
    float a; // quadtratic term
    vec3 color;
    float b; // linear term
    float c; // constant term
};
uniform PointLight u_PointLights[4]; // Point Lights
uniform int u_NumberOfPointLights;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};
uniform DirectionalLight u_DirectionalLight;
uniform bool u_DirectionalLightOn;

uniform vec3 u_AmbientLight;

uniform vec3 u_ViewPosition;

varying vec3 v_Position;

varying vec3 v_Normal;
varying vec2 v_TexCoordinate;
varying mat3 v_InverseTBN;

uniform vec4 u_InputValue;

// The entry point for our fragment shader.
void main()
{
    if (u_InputValue.w != 0.0) {
        gl_FragColor = vec4(u_InputValue.rgb, 0.0);
    } else {
        vec3 normal = v_Normal;
        gl_FragColor = vec4((normal + 1.0) * 0.5, 0.0);
    }
}
