#version 450

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in float v_scalar;
layout(location = 3) in uint v_palette_index;
layout(location = 4) in uint v_triangle_index;

layout(location = 0) out vec3 world_position;
layout(location = 1) out vec3 world_normal;
layout(location = 2) out float scalar;
layout(location = 3) flat out uint palette_index;
layout(location = 4) out vec3 barycenter;

layout(set = 1, binding = 0) uniform MatrixBlock {
	mat4 u_model;
	mat4 u_view;
	mat4 u_projection;
};
layout(set = 1, binding = 1) uniform PlanetBlock {
	vec3 u_palette[64];
	int u_overlay;
};

void main() {
	if (v_triangle_index == 0) {
		barycenter = vec3(1.0, 0.0, 0.0);
	} else if (v_triangle_index == 1) {
		barycenter = vec3(0.0, 1.0, 0.0);
	} else if (v_triangle_index == 2) {
		barycenter = vec3(0.0, 0.0, 1.0);
	}

	world_position = (u_model * vec4(v_position, 1.0)).xyz;
	world_normal = normalize((u_model * vec4(v_normal, 0.0)).xyz);
	palette_index = v_palette_index;
	scalar = v_scalar;
	gl_Position = u_projection * u_view * u_model * vec4(v_position, 1.0);
}