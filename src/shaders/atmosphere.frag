#version 450

layout(location = 0) in vec2 uv;
layout(location = 1) in vec2 ndc_position;

layout(location = 0) out vec4 fragColor;

void main() {
	fragColor = vec4(1.0, 1.0, 1.0, 0.2);
}