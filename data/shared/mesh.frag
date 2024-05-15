#version 460

out vec4 out_fragcolor;

in vec3 v2f_worldpos;
in vec3 v2f_normal;
in vec3 v2f_tangent;
in vec3 v2f_bitangent;
in vec2 v2f_uv;

//vec3 sunlight_dir = normalize(vec3(1, -0.4, 1));
//vec3 sunlight_color = vec3(2.4, 2.4, 2.4);
uniform vec3 sunlight_dir;
uniform vec3 sunlight_color;

vec3 viewpos;
vec3 viewdir;

vec3 surf_color;
vec3 surf_normal;
float surf_specular;
float surf_shininess;
float surf_emissive;
float surf_opacity;

void surf();

void main()
{
	surf_color = vec3(1.0, 0.0, 1.0);
	surf_normal = v2f_normal;
	surf_specular = 0.0;
	surf_shininess = 8.0;
	surf_emissive = 0.0;
	surf_opacity = 1.0;

	surf();

	float diff = max(dot(surf_normal, -sunlight_dir), 0.0);
	vec3 diffuse = sunlight_color * diff;
	vec3 ambient = vec3(0.15, 0.15, 0.15);

	out_fragcolor.rgb = surf_color.rgb * (ambient + diffuse);
	out_fragcolor.a = surf_opacity;
}


uniform sampler2D albedo_map;
uniform sampler2D normal_map;
uniform sampler2D specular_map;
uniform sampler2D emissive_map;
//uniform vec4 color_factor;

void surf()
{
	vec4 texel = texture(albedo_map, v2f_uv);
	surf_color = texel.rgb;
	surf_opacity = texel.a;
}
