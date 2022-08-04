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

// The entry point for our fragment shader.
void main()
{
    vec4 albedo = u_Albedo * texture2D(u_AlbedoMap, v_TexCoordinate);

    // vec3 normal = normalize(v_InverseTBN *
    //                 ((2.0 * texture2D(u_NormalMap, v_TexCoordinate).rgb) - 1.0));
    vec3 normal = v_Normal;

    float metallic = u_Metallic * texture2D(u_MetallicMap, v_TexCoordinate).r;
    float roughness = 1.0 - u_Roughness * texture2D(u_RoughnessMap, v_TexCoordinate).r;

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
    vec4 rawColor = vec4(lightContribution, 1.0) * albedo;

    // mat4 bayer4x4 = mat4(
    //     0.0, 8.0, 2.0, 10.0,
    //     12.0, 4.0, 14.0, 6.0,
    //     3.0, 11.0, 1.0, 9.0,
    //     15.0, 7.0, 13.0, 5.0
    // );
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

    // float M = bayer4x4[int(x)][int(y)];
    // float noise = M * (1.0/16.0) - 0.5;
    float noise = M;
    float spread = 0.05;

    vec3 dithered_color = rawColor.rgb + spread * M;

    float n = 8.0;
    vec3 compressed_color = floor(dithered_color * (n - 1.0) + 0.5) / (n - 1.0);
    gl_FragColor = vec4(compressed_color, rawColor.a);
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