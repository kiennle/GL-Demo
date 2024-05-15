#version 460

in vec3 position;

uniform mat4 matrix_vp;
uniform mat4 matrix_world;

mat4 matrix_wvp = matrix_world * matrix_vp;

void main()
{
	gl_Position = vec4(position, 1.0) * matrix_wvp;
}
