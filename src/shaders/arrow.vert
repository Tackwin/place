#version 450

layout(location = 0) in vec3 ignore;
layout(location = 1) in vec3 v_pos;
layout(location = 2) in vec3 v_dir;
layout(location = 3) in vec3 v_up;
layout(location = 4) in vec3 v_color;
layout(location = 5) in float v_scale;

layout(location = 0) out vec3 color;
layout(location = 1) out vec3 world_normal;
layout(location = 2) out vec3 world_position;
layout(location = 3) out vec2 debug1;
layout(location = 4) out vec3 debug2;
layout(location = 5) out vec3 debug3;
layout(location = 6) out vec3 debug4;
layout(location = 7) out vec3 debug5;
layout(location = 8) out vec3 debug6;

layout(set = 1, binding = 0) uniform MatrixBlock {
	mat4 u_model;
	mat4 u_view;
	mat4 u_projection;
};

void main() {
	vec2 v = vec2(0, 0);
	if (gl_VertexIndex == 0)
		v = vec2(0.0, -0.30);
	if (gl_VertexIndex == 1)
		v = vec2(0.5, -0.30);
	if (gl_VertexIndex == 2)
		v = vec2(0.0, +0.30);

	if (gl_VertexIndex == 3)
		v = vec2(0.5, -0.30);
	if (gl_VertexIndex == 4)
		v = vec2(0.5, +0.30);
	if (gl_VertexIndex == 5)
		v = vec2(0.0, +0.30);

	if (gl_VertexIndex == 6)
		v = vec2(0.5, -0.50);
	if (gl_VertexIndex == 7)
		v = vec2(1.0, 0.00);
	if (gl_VertexIndex == 8)
		v = vec2(0.5, +0.50);

	vec3 axis = vec3(v_dir.x * v.x, v_dir.y * v.x, v_dir.z * v.x) * 2;
	vec3 side = normalize(cross(v_dir, v_up));
	side = vec3(side.x * v.y, side.y * v.y, side.z * v.y);

	debug1 = v;
	debug2 = axis;
	debug3 = side;
	debug4 = v_pos + axis;
	debug5 = v_pos + side;
	debug6 = v_pos + (axis + side) * v_scale;

	vec3 model_position = debug6;
	world_position = (u_model * vec4(model_position, 1.0)).xyz;
	world_normal = normalize((u_model * vec4(v_up, 0.0)).xyz);
	color = v_color;
	gl_Position = u_projection * u_view * vec4(world_position, 1.0);
}