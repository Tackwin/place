#version 450

layout(location = 0) in vec3 world_position;
layout(location = 1) in vec3 world_normal;
layout(location = 2) in float scalar;
layout(location = 3) flat in uint palette_index;
layout(location = 4) in vec3 barycenter;

layout(location = 0) out vec4 fragColor;

layout(std140, set = 3, binding = 0) uniform MatrixBlock {
	mat4 u_model;
	mat4 u_view;
	mat4 u_projection;
};
layout(std140, set = 3, binding = 1) uniform PlanetBlock {
	vec3 u_palette[64];
	int u_overlay;
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

float saturate( float x ) { return clamp( x, 0.0, 1.0 ); }

vec3 viridis_quintic( float x )
{
	x = saturate( x );
	vec4 x1 = vec4( 1.0, x, x * x, x * x * x ); // 1 x x2 x3
	vec4 x2 = x1 * x1.w * x; // x4 x5 x6 x7
	return vec3(
		dot( x1.xyzw, vec4( +0.280268003, -0.143510503, +2.225793877, -14.815088879 ) ) + dot( x2.xy, vec2( +25.212752309, -11.772589584 ) ),
		dot( x1.xyzw, vec4( -0.002117546, +1.617109353, -1.909305070, +2.701152864 ) ) + dot( x2.xy, vec2( -1.685288385, +0.178738871 ) ),
		dot( x1.xyzw, vec4( +0.300805501, +2.614650302, -12.019139090, +28.933559110 ) ) + dot( x2.xy, vec2( -33.491294770, +13.762053843 ) ) );
}


void main() {

	vec3 light_position = vec3(0.0, 0.0, 0.0);
	vec3 light_direction = normalize(light_position - world_position);
	float light_intensity = max(dot(-world_normal, light_direction), 0.0);
	light_intensity = light_intensity + 0.1;

	vec3 color = u_palette[palette_index];
	color = color * light_intensity;

	vec3 baryd = max(abs(dFdx(barycenter)), abs(dFdy(barycenter)));
	vec3 baryd_min = min(baryd, 0.1);
	float baryd_factor = dot(baryd_min / baryd, vec3(1.0 / 3.0));
	vec3 baryf = smoothstep(baryd_min * 0, baryd_min * 1, barycenter);
	float b = min(min(baryf.x, baryf.y), baryf.z);

	color = mix(vec3(0.0), color, min(1.0, b + 1.5 * (1.0 - baryd_factor)));

	if (u_overlay > 0) {
		color = mix(color, viridis_quintic(scalar), 0.85);
	}
	fragColor = vec4(color, 1.0);

}
