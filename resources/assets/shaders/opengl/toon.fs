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
    gl_FragColor = vec4(lightContribution, 1.0) * albedo;
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
    // float stepWidth = 1.0;
    // float stepAmount = 2.0;
    float specularSize = 0.1;
    float specularFalloff = 0.5;

    vec3 lightDir = normalize(light.position - v_Position);

    float ndl = dot(normal, lightDir);
    // ndl = ndl / stepWidth;

    float intensity  = floor(ndl);
    // intensity = intensity / stepAmount;
    intensity = clamp(intensity, 0.0, 1.0);

    vec3 r = reflect(lightDir, normal);
    vec3 viewDir = normalize(u_ViewPosition - v_Position);
    float vdr = dot(viewDir, -r);
    float specFalloff = dot(viewDir, normal);
    specFalloff = pow(specFalloff, specularFalloff);
    vdr = vdr * specFalloff;

    float dist = distance(light.position, v_Position);
    float attenuation = min(1.0 / (light.c + light.b * dist + light.a * dist * dist), 1.0);

    float spec = min(pow((1.0 - roughness) * 5.0 * vdr, 16.0), 1.0);

    float lightVal = clamp(intensity + spec, 0.0, 1.0);
    lightVal = lightVal > 0.5 ? 1.0 : 0.0;

    return lightVal * light.color * attenuation * albedo;
}

vec3 CalculateDirectionalLight(DirectionalLight light,
                               vec3  albedo,
                               vec3  normal,
                               float metallic,
                               float roughness) {
    // diffuse shading
    float diff = max(dot(normal, -light.direction), 0.0) < 0.5 ? 0.0 : 1.0;
    // specular shading
    vec3 r = reflect(light.direction, normal);
    float shininess = 8.0;

    vec3 viewDir = normalize(u_ViewPosition - v_Position);
    float spec = (1.0 - roughness) * pow(max(dot(r, viewDir), 0.0), shininess) < 0.5 ? 0.0 : 1.0;
    // combine results
    vec3 diffuse  = light.color * diff * albedo;
    vec3 specular = vec3(1) * spec;

    // float lightVal = clamp(diffuse + spec, 0.0, 1.0);
    // lightVal = lightVal > 0.5 ? 1.0 : 0.0;

    return diffuse + specular;
}
