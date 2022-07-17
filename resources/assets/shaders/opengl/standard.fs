precision mediump float;        // Set the default precision to medium. We don't need as high of a
                                // precision in the fragment shader.

uniform sampler2D u_Albedo;    // The input texture.

struct PointLight {
    vec3 position;
    float a; // quadtratic term
    vec3 color;
    float b; // linear term
    float c; // constant term
};
uniform PointLight u_PointLights[4]; // Point Lights
uniform int u_NumberOfPointLights;

uniform vec3 u_ViewPosition;

varying vec3 v_Position;        // Interpolated position for this fragment.

varying vec3 v_Normal;          // Interpolated normal for this fragment.
varying vec2 v_TexCoordinate;   // Interpolated texture coordinate per fragment.

const float PI = 3.14159265359;

vec3 calculatePointLight(PointLight light, vec3 albedo);

// The entry point for our fragment shader.
void main()
{
    vec4 albedo = texture2D(u_Albedo, v_TexCoordinate);
    vec3 lightContribution = vec3(0.0);
    // Add ambient
    lightContribution += 0.3;
    // Add point lights
    for (int i = 0; i < 4; ++i) {
        // Constant loop count work-around
        if (i < u_NumberOfPointLights) {
            lightContribution += calculatePointLight(u_PointLights[i],
                                                     albedo.rgb);
        }
    }

    // Multiply the color by the diffuse illumination level and texture value to get final output color.
    // gl_FragColor = (v_Color * diffuse * texture2D(u_Texture, v_TexCoordinate));
    gl_FragColor = vec4(lightContribution, 1.0) * albedo;
    // gl_FragColor = vec4(1.0) * albedo;
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

vec3 calculatePointLight(PointLight light, vec3 albedo) {
    // // Will be used for attenuation.
    // float distance = length(light.position - v_Position);

    // // Get a lighting direction vector from the light to the vertex.
    // vec3 lightVector = normalize(light.position - v_Position);

    // // Calculate the dot product of the light vector and vertex normal. If the normal and light vector are
    // // pointing in the same direction then it will get max illumination.
    // float diffuse = max(dot(v_Normal, lightVector), 0.0);

    // // Add attenuation.
    // diffuse = diffuse * (1.0 / (1.0 + (0.10 * distance)));

    // return light.color * diffuse;

    // TODO: retrieve these values properly
    float roughness = 0.5;
    float metallic = 0.5;
    vec3 N = v_Normal;
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 V = normalize(u_ViewPosition - v_Position);
    vec3 L = normalize(light.position - v_Position);
    vec3 H = normalize(V + L);

    float dist = length(light.position  - v_Position);
    float a =  light.a * dist * dist;
    float b =  light.c * dist;
    float c =  light.c;
    float attenuation = 1.0 / (a + b + c);
    vec3 radiance = light.color * attenuation;

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N,L), 0.0);
    vec3 specular = numerator / max(denominator, 0.001);

    float NdotL = max(dot(N, L), 0.0);
    vec3 contribution = (kD * albedo / PI + specular) * radiance * NdotL;

    return contribution;
}
