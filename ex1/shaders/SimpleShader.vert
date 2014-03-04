#version 330

in vec4 position;
in float faultVal;

uniform mat4 wvp;

void main()
{
	/*mat4 faultMat = mat4(	1.0, 0.0, 0.0, 0.0,
							0.0, 1.0, 0.0, faultVal,
							0.0, 0.0, 1.0, 0.0,
							0.0, 0.0, 0.0, 1.0)*/
	gl_Position = wvp * position;
}
