#version 460

in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec2 uv;

out vec3 v2f_worldpos;
out vec3 v2f_normal;
out vec3 v2f_tangent;
out vec3 v2f_bitangent;
out vec2 v2f_uv;

uniform mat4 matrix_vp;
uniform mat4 matrix_world;
uniform mat4 matrix_inv_world;

mat4 matrix_wvp = matrix_world * matrix_vp;
mat4 matrix_inv_transpose_world = transpose(matrix_inv_world);

void main()
{
	v2f_worldpos = (vec4(position, 1.0) * matrix_world).xyz;
	v2f_normal = normalize(normal * mat3(matrix_inv_transpose_world));
	v2f_tangent = normalize(tangent * mat3(matrix_inv_transpose_world));
	v2f_bitangent = normalize(cross(v2f_normal, v2f_tangent));
	v2f_uv = uv;
	gl_Position = vec4(position, 1.0) * matrix_wvp;
}
