#version 450

layout(location = 0) in vec3 world_position;
layout(location = 1) in vec3 world_normal;
layout(location = 2) in float height;

layout(location = 0) out vec4 fragColor;

layout(set = 3, binding = 0) uniform MatrixBlock {
	mat4 u_model;
	mat4 u_view;
	mat4 u_projection;
};

float stepify(float x) {
	if (x < 0.25)
		return 0.125;
	if (x < 0.50)
		return 0.375;
	if (x < 0.75)
		return 0.625;
	return 0.875;
}

float ease(float x) {
	if (x < -1)
		return -1;
	if (x > 1)
		return 1;
	return x * x * x;
}

void main() {
	vec3 light_direction = normalize(vec3(1.0, 0.0, 1.0));
	float light_intensity = max(dot(-world_normal, light_direction), 0.0);
	
	float t = height;
	if (t < -1)
		t = -1;
	if (t > 1)
		t = 1;
	t = ease(height) / 2 + 0.5;

	vec3 deep_ocean_color = vec3(0 / 255.0, 26 / 255.0, 51 / 255.0);
	vec3 shallow_ocean_color = vec3(0 / 255.0, 102 / 255.0, 204 / 255.0);
	vec3 beach_color = vec3(249 / 255.0, 209 / 255.0, 153 / 255.0);
	vec3 forest_color = vec3(34 / 255.0, 49 / 255.0, 29 / 255.0);
	vec3 peak_color = vec3(1.0, 1.0, 1.0);

	vec3 color = peak_color;
	if (0.00 <= t && t < 0.15) {
		float u = t / 0.15;
		u = stepify(u);
		color = mix(deep_ocean_color, shallow_ocean_color, u);
	}
	if (0.15 <= t && t < 0.35) {
		float u = (t - 0.15) / 0.20;
		u = stepify(u);
		color = mix(shallow_ocean_color, beach_color, u);
	}
	if (0.35 <= t && t < 0.45) {
		float u = (t - 0.35) / 0.10;
		u = stepify(u);
		color = mix(beach_color, forest_color, u);
	}
	if (0.45 <= t && t <= 0.80) {
		float u = (t - 0.45) / 0.35;
		u = stepify(u);
		color = mix(forest_color, peak_color, u);
	}
	if (0.80 < t && t <= 1.00) {
		float u = (t - 0.80) / 0.20;
		u = stepify(u);
		color = mix(peak_color, vec3(1.0, 1.0, 1.0), u);
	}

	color *= (0.5 + 0.5 * light_intensity);

	fragColor = vec4(vec3(color), 1.0);
}
