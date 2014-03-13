#version 330

uniform vec4 fillColor;

layout(location = 0) out vec4 outColor;

//MRT: layout(location = 1) out vec4 outNormal;

void main()
{
	outColor = fillColor;
}
