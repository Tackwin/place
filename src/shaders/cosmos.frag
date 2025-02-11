#version 450

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 fragColor;

layout(std140, set = 3, binding = 0) uniform MatrixBlock {
	mat4 u_model;
	mat4 u_view;
	mat4 u_projection;
};

struct Star {
	float position_x;
	float position_y;
	float position_z;
	float color_x;
	float color_y;
	float color_z;
	float radius;
};

layout(std140, set = 3, binding = 1) uniform BufferBlock {
	vec3 empty_color;
};
vec2 mod2(inout vec2 p, vec2 size) {
  vec2 c = floor((p + size*0.5)/size);
  p = mod(p + size*0.5,size) - size*0.5;
  return c;
}
vec3 toSpherical(vec3 p) {
	float r   = length(p);
	float t   = acos(p.z/r);
	float ph  = atan(p.y, p.x);
	return vec3(r, t, ph);
}

vec3 world_ray() {
	vec3 ndc = vec3(uv * 2.0 - vec2(1.0), -1.0);
	vec4 ray_eye = inverse(u_projection) * vec4(ndc, 1.0);
	ray_eye.z = -1.0;
	ray_eye.w = 0.0;
	vec3 ray_world = vec3(inverse(u_view) * ray_eye);
	return normalize(ray_world);
}

vec3 grid(vec3 ro, vec3 rd, vec2 sp) {
  const float m = 1.0;

  const vec2 dim = vec2(1.0/8.0*3.1415926);
  vec2 pp = sp;
  vec2 np = mod2(pp, dim);

  vec3 col = vec3(0.0);

  float y = sin(sp.x);
  float d = min(abs(pp.x), abs(pp.y*y));
  
  float aa = 2.0/1000.0;
  
  col += 2.0*vec3(0.5, 0.5, 1.0)*exp(-2000.0*max(d-0.00025, 0.0));
  
  return 0.25*tanh(col);
}

void main() {
	vec3 ray_world = world_ray();

	vec2 spher = toSpherical(ray_world).yz;
	vec3 col = grid(vec3(0.0), ray_world, spher);

	fragColor = vec4(col, 1.0);
}
