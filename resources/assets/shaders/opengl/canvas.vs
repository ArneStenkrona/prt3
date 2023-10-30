#version 300 es

layout(location = 0) in vec4 a_PosUV;
layout(location = 1) in vec4 a_Color;

out vec2 v_UV;
out vec4 v_Color;

void main(){
	v_UV = a_PosUV.zw;
	gl_Position = vec4(a_PosUV.xy, 0.0, 1.0);
	v_Color = a_Color;
}
