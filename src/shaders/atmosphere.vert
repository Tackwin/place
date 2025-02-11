#version 450

layout(location = 0) in uint v_index;

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
layout(location = 1) out vec2 ndc_position;

void main() {
	if (v_index == 0) {
		gl_Position = vec4(-1.0, -1.0, 0.0, 1.0);
		uv = vec2(0.0, 0.0);
	} else if (v_index == 1) {
		gl_Position = vec4(1.0, -1.0, 0.0, 1.0);
		uv = vec2(1.0, 0.0);
	} else if (v_index == 2) {
		gl_Position = vec4(1.0, 1.0, 0.0, 1.0);
		uv = vec2(1.0, 1.0);
	} else if (v_index == 3) {
		gl_Position = vec4(-1.0, 1.0, 0.0, 1.0);
		uv = vec2(0.0, 1.0);
	}

	ndc_position = gl_Position.xy / gl_Position.w;
}