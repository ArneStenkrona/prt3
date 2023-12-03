#version 300 es

precision mediump float;

uniform float u_NearPlane;
uniform float u_FarPlane;

uniform sampler2D u_DepthMap;
uniform sampler2D u_Texture;

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

in vec3 v_Position;
in vec2 v_UV;
in vec4 v_Color;
in vec2 v_screenUV;

const float PI = 3.14159265359;

vec3 calculatePointLight(PointLight light,
                         vec3 albedo,
                         vec3 normal,
                         float metallic,
                         float roughness);

vec3 CalculateDirectionalLight(DirectionalLight light,
                               vec3  albedo,
                               vec3  normal,
                               float metallic,
                               float roughness);

layout(location=0) out vec4 outAccumColor;
layout(location=1) out float outAccumAlpha;

float weight(float z, float a) {
    return clamp(pow(min(1.0, a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - z * 0.9, 3.0), 1e-2, 3e3);
}

float linearize_depth(float z) {
    float n = u_NearPlane;
    float f = u_FarPlane;
    return (2.0 * n) / (f + n - z * (f - n));
}

void main() {
    vec4 albedo = v_Color * texture(u_Texture, v_UV);

    // fade close to opaque geometry
    float sample_d = linearize_depth(texture(u_DepthMap, v_screenUV).r);
    float d = linearize_depth(gl_FragCoord.z);
    float depth_lim = 0.005;
    float d_diff = clamp(sample_d - d, 0.0, depth_lim);
    float depth_fade = d_diff / depth_lim;
    albedo.a *= depth_fade;

    vec3 normal = normalize(u_ViewPosition - v_Position);

    float metallic = 0.0;
    float roughness = 1.0;

    vec3 lightContribution = u_AmbientLight;
    // Add point lights
    for (int i = 0; i < 4; ++i) {
        // Constant loop count work-around
        if (i < u_NumberOfPointLights) {
            lightContribution += calculatePointLight(u_PointLights[i],
                                                     albedo.rgb,
                                                     normal,
                                                     metallic,
                                                     roughness);
        }
    }

    if (u_DirectionalLightOn) {
        lightContribution += CalculateDirectionalLight(
            u_DirectionalLight,
            albedo.rgb,
            normal,
            metallic,
            roughness
        );
    }

    vec3 color = lightContribution * albedo.rgb;

    color.rgb *= albedo.a;
    float w = weight(gl_FragCoord.z, albedo.a);

    outAccumColor = vec4(color * w, albedo.a);
    outAccumAlpha = albedo.a * w;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a*a;
    float NdotH = max(dot(N,H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 calculatePointLight(PointLight light,
                         vec3 albedo,
                         vec3 normal,
                         float metallic,
                         float roughness) {
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 V = normalize(u_ViewPosition - v_Position);
    vec3 L = normalize(light.position - v_Position);
    vec3 H = normalize(V + L);

    float dist = length(light.position  - v_Position);
    float a =  light.a * dist * dist;
    float b =  light.b * dist;
    float c =  light.c;
    float attenuation = 1.0 / (a + b + c);
    vec3 radiance = light.color * attenuation;

    float NDF = DistributionGGX(normal, H, roughness);
    float G = GeometrySmith(normal, V, L, roughness);
    vec3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal,L), 0.0);
    vec3 specular = numerator / max(denominator, 0.001);

    float NdotL = max(dot(normal, L), 0.0);
    vec3 contribution = (kD * albedo / PI + specular) * radiance * NdotL;

    return contribution;
}

vec3 CalculateDirectionalLight(DirectionalLight light,
                               vec3  albedo,
                               vec3  normal,
                               float metallic,
                               float roughness) {
    vec3 direction = light.direction;
    vec3 color = 10.0 * light.color;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 V = normalize(u_ViewPosition - v_Position);
    vec3 L = normalize(-direction);
    vec3 H = normalize(V + L);

    vec3 radiance = color;

    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    float NDF = DistributionGGX(normal, H, roughness);
    float G = GeometrySmith(normal, V, L, roughness);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0);
    vec3 specular = numerator / max(denominator, 0.001);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;

    kD *= 1.0 - metallic;

    float NdotL = max(dot(normal, L), 0.0);
    vec3 contribution = (kD * albedo / PI + specular) * radiance * NdotL;

    return contribution;
}