uniform mat4 u_MMatrix;        // A constant representing the model matrix.
uniform mat4 u_MVPMatrix;       // A constant representing the combined model/view/projection matrix.
uniform mat4 u_MVMatrix;        // A constant representing the combined model/view matrix.
uniform mat3 u_InvTposMMatrix;

attribute vec3 a_Position;      // Per-vertex position information we will pass in.
attribute vec3 a_Normal;         // Per-vertex color information we will pass in.
attribute vec2 a_TexCoordinate; // Per-vertex texture coordinate information we will pass in.
attribute vec3 a_Tangent;        // Per-vertex tangent information we will pass in.
attribute vec3 a_Bitangent;        // Per-vertex bitangent information we will pass in.

varying vec3 v_Position;        // This will be passed into the fragment shader.
varying vec3 v_Normal;          // This will be passed into the fragment shader.
varying vec2 v_TexCoordinate;   // This will be passed into the fragment shader.

// The entry point for our vertex shader.
void main()
{
    // Transform the vertex into eye space.
    // v_Position = vec3(u_MVMatrix * vec4(a_Position, 1.0));
    v_Position = vec3(u_MMatrix * vec4(a_Position, 1.0));

    // Pass through the color.
    // v_Color = a_Color;

    // Pass through the texture coordinate.
    v_TexCoordinate = a_TexCoordinate;

    // Transform the normal's orientation into world space.
    v_Normal = u_InvTposMMatrix * a_Normal;

    // gl_Position is a special variable used to store the final position.
    // Multiply the vertex by the matrix to get the final point in normalized screen coordinates.
    gl_Position = u_MVPMatrix * vec4(a_Position, 1.0);
}
