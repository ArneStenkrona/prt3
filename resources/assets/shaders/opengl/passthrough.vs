#version 300 es

layout(location = 0) in vec3 a_Position;

out vec2 v_TexCoordinate;

void main(){
	gl_Position = vec4(a_Position, 1);
	v_TexCoordinate = (a_Position.xy + vec2(1.0, 1.0)) / 2.0;
}
