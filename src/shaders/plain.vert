#version 450

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in float v_height;

layout(location = 0) out vec3 world_position;
layout(location = 1) out vec3 world_normal;
layout(location = 2) out float height;

layout(set = 1, binding = 0) uniform MatrixBlock {
	mat4 u_model;
	mat4 u_view;
	mat4 u_projection;
};

void main() {
	world_position = (u_model * vec4(v_position, 1.0)).xyz;
	world_normal = normalize((u_model * vec4(v_normal, 0.0)).xyz);
	height = v_height;
	gl_Position = u_projection * u_view * u_model * vec4(v_position, 1.0);
}