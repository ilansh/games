#version 330

in vec4 position;
in float faultVal;

uniform mat4 wvp;

void main()
{
	gl_Position = wvp * position;
}
