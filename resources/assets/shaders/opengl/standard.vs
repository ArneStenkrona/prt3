uniform mat4 u_MMatrix;
uniform mat4 u_MVPMatrix;
uniform mat4 u_MVMatrix;
uniform mat3 u_InvTposMMatrix;

attribute vec3 a_Position;
attribute vec3 a_Normal;
attribute vec2 a_TexCoordinate;
attribute vec3 a_Tangent;
attribute vec3 a_Bitangent;

varying vec3 v_Position;
varying vec3 v_Normal;
varying vec2 v_TexCoordinate;
varying mat3 v_InverseTBN;

// The entry point for our vertex shader.
void main()
{
    // Transform the vertex into eye space.
    v_Position = vec3(u_MMatrix * vec4(a_Position, 1.0));

    // Pass through the texture coordinate.
    v_TexCoordinate = a_TexCoordinate;

    // Transform the normal's orientation into world space.
    v_Normal = u_InvTposMMatrix * a_Normal;

    vec3 t = normalize(u_InvTposMMatrix * a_Tangent);
    vec3 b = normalize(u_InvTposMMatrix * a_Bitangent);
    vec3 n = normalize(u_InvTposMMatrix * a_Normal);

    v_InverseTBN = mat3(t,b,n);

    // gl_Position is a special variable used to store the final position.
    // Multiply the vertex by the matrix to get the final point in normalized screen coordinates.
    gl_Position = u_MVPMatrix * vec4(a_Position, 1.0);
}
