#version 450

layout(location = 0) in vec3 color;
layout(location = 1) in vec3 world_normal;
layout(location = 2) in vec3 world_position;
layout(location = 3) in vec2 debug1;
layout(location = 4) in vec3 debug2;
layout(location = 5) in vec3 debug3;
layout(location = 6) in vec3 debug4;
layout(location = 7) in vec3 debug5;
layout(location = 8) in vec3 debug6;


layout(location = 0) out vec4 fragColor;

layout(set = 3, binding = 0) uniform MatrixBlock {
	mat4 u_model;
	mat4 u_view;
	mat4 u_projection;
};

void main() {

	fragColor = vec4(color, 1.0);
}
