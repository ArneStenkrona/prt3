#version 300 es

uniform mat4 u_MMatrix;
uniform mat4 u_MVPMatrix;
uniform mat4 u_MVMatrix;
uniform mat3 u_InvTposMMatrix;

uniform mat4 u_Bones[100];

uniform float u_NearPlane;
uniform float u_FarPlane;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoordinate;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;
layout(location = 5) in uvec4 a_BoneIDs;
layout(location = 6) in vec4 a_BoneWeights;

out vec3 v_Position;
out vec3 v_Normal;
out vec2 v_TexCoordinate;
out mat3 v_InverseTBN;

void main() {
    mat4 boneTransform = mat4(1.0);
    float weightSum = a_BoneWeights[0] + a_BoneWeights[1] + a_BoneWeights[2] + a_BoneWeights[3];
    if (weightSum > 0.0) {
        boneTransform  = u_Bones[a_BoneIDs[0]] * a_BoneWeights[0];
        boneTransform += u_Bones[a_BoneIDs[1]] * a_BoneWeights[1];
        boneTransform += u_Bones[a_BoneIDs[2]] * a_BoneWeights[2];
        boneTransform += u_Bones[a_BoneIDs[3]] * a_BoneWeights[3];
    }

    mat3 invtposBone = inverse(transpose(mat3(boneTransform)));

    vec4 bonedPos = boneTransform * vec4(a_Position, 1.0);
    vec3 bonedNormal = invtposBone * a_Normal;

    v_Position = vec3(u_MMatrix * bonedPos);

    v_TexCoordinate = a_TexCoordinate;

    v_Normal = u_InvTposMMatrix * bonedNormal;

    vec3 t = normalize(u_InvTposMMatrix * invtposBone * a_Tangent);
    vec3 b = normalize(u_InvTposMMatrix * invtposBone * a_Bitangent);
    vec3 n = normalize(u_InvTposMMatrix * invtposBone * a_Normal);

    v_InverseTBN = mat3(t,b,n);

    gl_Position = u_MVPMatrix * bonedPos;
}
