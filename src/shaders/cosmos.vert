#version 450

layout(location = 0) in vec2 pos;

layout(std140, set = 1, binding = 0) uniform MatrixBlock {
	mat4 u_model;
	mat4 u_view;
	mat4 u_projection;
};
layout(std140, set = 1, binding = 1) uniform BufferBlock {
	vec3 empty_color;
};
layout(location = 0) out vec2 uv;

void main() {
	uv = pos;
	gl_Position = vec4(pos * 2 - vec2(1.0), 1.0, 1.0);
}