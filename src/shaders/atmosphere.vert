#version 450

layout(location = 0) in vec2 pos;

layout(set = 1, binding = 0) uniform MatrixBlock {
	mat4 u_view;
	mat4 u_projection;
};
layout(set = 1, binding = 1) uniform AtmosphereBlock {
	vec3 u_planet_world_position;
	vec3 u_planet_world_axis;
	vec3 u_planet_radius;

	float u_atmosphere_thickness;
};

layout(location = 0) out vec2 uv;

void main() {
	uv = pos;
	gl_Position = vec4(pos * 2 - vec2(1.0), 1.0, 1.0);
}