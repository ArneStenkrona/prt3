#version 300 es

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

uniform uint u_NodeData;
uniform bool u_Selected;

uniform mat4 u_VPMatrix;

in vec3 v_Position;
in vec3 v_Normal;
in vec2 v_TexCoordinate;
in mat3 v_InverseTBN;

const float PI = 3.14159265359;

const float toon_steps = 1.0;

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

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec4 outMetadata;

void main() {
    vec4 albedo = u_Albedo * texture(u_AlbedoMap, v_TexCoordinate);

    // vec3 normal = normalize(v_InverseTBN *
    //                 ((2.0 * texture(u_NormalMap, v_TexCoordinate).rgb) - 1.0));
    vec3 normal = v_Normal;

    float metallic = u_Metallic * texture(u_MetallicMap, v_TexCoordinate).r;
    float roughness = u_Roughness * texture(u_RoughnessMap, v_TexCoordinate).r;

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

    outColor = lightContribution * albedo.rgb;
    outNormal = 0.5 * normal + 0.5;

    outMetadata.r = float(u_NodeData % uint(256)) / 255.0;
    outMetadata.g = float((u_NodeData / uint(256)) % uint(256)) / 255.0;
    outMetadata.b = float((u_NodeData / uint(65536)) % uint(256)) / 255.0;
    outMetadata.a = float((u_NodeData / uint(16777216))) / 255.0;
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

float toon_quantize(float f) {
    if (toon_steps >= 0.0) return f;
    return floor(f * (toon_steps - 1.0) + 0.5) / (toon_steps - 1.0);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = toon_quantize(max(dot(N, L), 0.0));
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

    float NdotL = toon_quantize(max(dot(normal, L), 0.0));
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

    float NdotL = toon_quantize(max(dot(normal, L), 0.0));
    vec3 contribution = (kD * albedo / PI + specular) * radiance * NdotL;

    return contribution;
}