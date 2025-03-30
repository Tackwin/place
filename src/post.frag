#version 450

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fragColor;

layout(std140, set = 3, binding = 0) uniform UniformBlock {
	sampler2D hdr;

	float exposure;
	float startCompression;
	float desaturation;
};

vec3 sampleHDR(vec2 uv) {
	return texture(hdr, uv).rgb;
}

vec3 CommerceToneMapping(vec3 color) {
	color *= exposure;
	float x = min(color.r, min(color.g, color.b));
	float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;
	color -= offset;

	float peak = max(color.r, max(color.g, color.b));
	if (peak < startCompression)
		return color;
	
	float d = 1.0 - startCompression;
	float newPeak = 1.0 - d * d / (peak + d - startCompression);
	color *= newPeak / peak;

	float g = 1.0 - 1.0 / (desaturation * (peak - newPeak) + 1.0);
	return mix(color, newPeak * vec3(1.0), g);
}

void main() {
	fragColor = vec4(0.1, 0.1, 0.1, 0.2);
}