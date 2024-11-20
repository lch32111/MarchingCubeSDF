#version 330 core
layout (location = 0) in vec3 apos;
layout (location = 1) in vec3 anormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vpos;
out vec3 vnormal;

void main()
{
	vpos = (model * vec4(apos, 1.0)).xyz;
	vnormal = normalize(vec3((model * vec4(anormal, 0.0)).xyz));

	gl_Position = projection * view * vec4(vpos, 1.0);
}