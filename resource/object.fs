// shadertype=glsl
#version 330 core

in vec3 vpos;
in vec3 vnormal;

out vec4 frag_color;

uniform vec3 cam_pos;

uniform vec3 sun_dir;
uniform vec3 sun_ambient;
uniform vec3 sun_diffuse;
uniform vec3 sun_specular;

uniform vec3 mat_ambient;
uniform vec3 mat_diffuse;
uniform vec3 mat_specular;
uniform float mat_shininess;

void main()
{
	vec3 light_dir = normalize(sun_dir);
	vec3 view_dir = normalize(cam_pos - vpos);

	vec3 ambient_color = sun_ambient * mat_ambient;
	vec3 normal = normalize(vnormal);
	
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 diffuse_color = sun_diffuse * mat_diffuse * diff;

	vec3 halfway_dir = normalize(light_dir + view_dir);
	float spec = pow(max(dot(normal, halfway_dir), 0.0), mat_shininess);
	vec3 specular_color = sun_specular * mat_specular * spec;

	vec4 lighting_color = vec4(ambient_color + diffuse_color + specular_color, 1.0);

	frag_color = lighting_color;
}