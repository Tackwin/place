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

// BLACK BODY TEMPERATURE

#define LAMBDA_RED 780.0
#define LAMBDA_VIOLET 380.0

#define NMTOM(a) (a)*pow(10.0,-9.0)
#define MTONM(a) (a)*pow(10.0,+9.0)

const mat3 NTSC_Chromaticity = 
mat3(
	0.67, 0.21, 0.14,
	0.33, 0.71, 0.08,
	0.00, 0.08, 0.78
);
const vec3 NTSC_White = vec3 (0.3101,0.3162, 1.0 - (0.3101 + 0.3162) );

// Calculate spectra of a blackbody when a given wavelength is plugged into it
float bb_spectrum(float wavelength, float bbTemp)
{
	float wlm = NMTOM(wavelength);

	return (3.74183e-16 * pow(wlm, -5.0)) / (exp(1.4388e-2 / (wlm * bbTemp)) - 1.0);
}

vec3 XYZ(float bbTemp) {

	// delta lambda is equal to 5nm
	// stolen from https://www.fourmilab.ch/documents/specrend/
	const int cie_colour_match_length = 81;
	vec3 cie_colour_match[81] = vec3[](
		vec3(0.0014,0.0000,0.0065), vec3(0.0022,0.0001,0.0105), vec3(0.0042,0.0001,0.0201),
		vec3(0.0076,0.0002,0.0362), vec3(0.0143,0.0004,0.0679), vec3(0.0232,0.0006,0.1102),
		vec3(0.0435,0.0012,0.2074), vec3(0.0776,0.0022,0.3713), vec3(0.1344,0.0040,0.6456),
		vec3(0.2148,0.0073,1.0391), vec3(0.2839,0.0116,1.3856), vec3(0.3285,0.0168,1.6230),
		vec3(0.3483,0.0230,1.7471), vec3(0.3481,0.0298,1.7826), vec3(0.3362,0.0380,1.7721),
		vec3(0.3187,0.0480,1.7441), vec3(0.2908,0.0600,1.6692), vec3(0.2511,0.0739,1.5281),
		vec3(0.1954,0.0910,1.2876), vec3(0.1421,0.1126,1.0419), vec3(0.0956,0.1390,0.8130),
		vec3(0.0580,0.1693,0.6162), vec3(0.0320,0.2080,0.4652), vec3(0.0147,0.2586,0.3533),
		vec3(0.0049,0.3230,0.2720), vec3(0.0024,0.4073,0.2123), vec3(0.0093,0.5030,0.1582),
		vec3(0.0291,0.6082,0.1117), vec3(0.0633,0.7100,0.0782), vec3(0.1096,0.7932,0.0573),
		vec3(0.1655,0.8620,0.0422), vec3(0.2257,0.9149,0.0298), vec3(0.2904,0.9540,0.0203),
		vec3(0.3597,0.9803,0.0134), vec3(0.4334,0.9950,0.0087), vec3(0.5121,1.0000,0.0057),
		vec3(0.5945,0.9950,0.0039), vec3(0.6784,0.9786,0.0027), vec3(0.7621,0.9520,0.0021),
		vec3(0.8425,0.9154,0.0018), vec3(0.9163,0.8700,0.0017), vec3(0.9786,0.8163,0.0014),
		vec3(1.0263,0.7570,0.0011), vec3(1.0567,0.6949,0.0010), vec3(1.0622,0.6310,0.0008),
		vec3(1.0456,0.5668,0.0006), vec3(1.0026,0.5030,0.0003), vec3(0.9384,0.4412,0.0002),
		vec3(0.8544,0.3810,0.0002), vec3(0.7514,0.3210,0.0001), vec3(0.6424,0.2650,0.0000),
		vec3(0.5419,0.2170,0.0000), vec3(0.4479,0.1750,0.0000), vec3(0.3608,0.1382,0.0000),
		vec3(0.2835,0.1070,0.0000), vec3(0.2187,0.0816,0.0000), vec3(0.1649,0.0610,0.0000),
		vec3(0.1212,0.0446,0.0000), vec3(0.0874,0.0320,0.0000), vec3(0.0636,0.0232,0.0000),
		vec3(0.0468,0.0170,0.0000), vec3(0.0329,0.0119,0.0000), vec3(0.0227,0.0082,0.0000),
		vec3(0.0158,0.0057,0.0000), vec3(0.0114,0.0041,0.0000), vec3(0.0081,0.0029,0.0000),
		vec3(0.0058,0.0021,0.0000), vec3(0.0041,0.0015,0.0000), vec3(0.0029,0.0010,0.0000),
		vec3(0.0020,0.0007,0.0000), vec3(0.0014,0.0005,0.0000), vec3(0.0010,0.0004,0.0000),
		vec3(0.0007,0.0002,0.0000), vec3(0.0005,0.0002,0.0000), vec3(0.0003,0.0001,0.0000),
		vec3(0.0002,0.0001,0.0000), vec3(0.0002,0.0001,0.0000), vec3(0.0001,0.0000,0.0000),
		vec3(0.0001,0.0000,0.0000), vec3(0.0001,0.0000,0.0000), vec3(0.0000,0.0000,0.0000)
	);
	
	vec3 xyz = vec3(0.0);
	float lambda = LAMBDA_VIOLET;
	int i = 0;
	while(lambda <= LAMBDA_RED) {
		// get the specular intensity of the color matching function
		float spec_intens = bb_spectrum(lambda,bbTemp);
		xyz += spec_intens*cie_colour_match[i];
		i++;
		lambda += 5.0;
	}
	
	return xyz;
}

vec3 ChromaticityValue(vec3 CIE) {
	float x = CIE.x / (CIE.x + CIE.y + CIE.z);
	float y = CIE.y / (CIE.x + CIE.y + CIE.z);
	float z = CIE.z / (CIE.x + CIE.y + CIE.z);
	
	return vec3(x,y,z);
}

vec3 constrainRGB(vec3 col) {
	vec3 ret = col;
	float w = -min(0.0, min(col.r,min(col.g,col.b)));
	
	w = (0.0 < col.r) ? 0.0 : col.r;
	w = (0.0 < col.g) ? w : col.g;
	w = (0.0 < col.b) ? w : col.b;
	w = -w;
	
	if(w > 0.0) {
		ret.r += w;
		ret.g += w;
		ret.b += w;
	}
	
	return ret;
}

vec3 normalizeRGB(vec3 col) {
#define MAX(a , b) ((a) > (b)) ? (a) : (b)
	float greatest = MAX(col.r, MAX(col.g, col.b));
	
	if(greatest > 0.0) {
		return col/greatest;
	}
	
	return col;

}

vec3 getJVector(vec3 w, mat3 table, vec3 xyz) {
	// linear algebra moment
	
	float xc = xyz.x;
	float yc = xyz.y;
	float zc = xyz.z;

	float xr, yr, zr, xg, yg, zg, xb, yb, zb;
	float xw, yw, zw;
	float rx, ry, rz, gx, gy, gz, bx, by, bz;
	float rw, gw, bw;
	
	// values from table
	xr = table[0][0];
	yr = table[1][0];
	zr = 1.0 - (xr + yr);
	
	xg = table[0][1];
	yg = table[1][1];
	zg = 1.0 - (xg + yg);
	
	xb = table[0][2];
	yb = table[1][2];
	zb = 1.0 - (xb + yb);
	
	// white values
	xw = w.x;
	yw = w.y;
	zw = w.z;
	
	// More declarations
	rx = (yg * zb) - (yb * zg);  ry = (xb * zg) - (xg * zb);  rz = (xg * yb) - (xb * yg);
	gx = (yb * zr) - (yr * zb);  gy = (xr * zb) - (xb * zr);  gz = (xb * yr) - (xr * yb);
	bx = (yr * zg) - (yg * zr);  by = (xg * zr) - (xr * zg);  bz = (xr * yg) - (xg * yr);
	
	// Declaring the white factor
	rw = ((rx * xw) + (ry * yw) + (rz * zw)) / yw;
	gw = ((gx * xw) + (gy * yw) + (gz * zw)) / yw;
	bw = ((bx * xw) + (by * yw) + (bz * zw)) / yw;
	
	// scale by the white factor
	rx = rx / rw;  ry = ry / rw;  rz = rz / rw;
	gx = gx / gw;  gy = gy / gw;  gz = gz / gw;
	bx = bx / bw;  by = by / bw;  bz = bz / bw;
	
	float r = (rx * xc) + (ry * yc) + (rz * zc);
	float g = (gx * xc) + (gy * yc) + (gz * zc);
	float b = (bx * xc) + (by * yc) + (bz * zc);
	
	return vec3(r,g,b);
}

// The almighty
vec3 getRGB(float bbTemp) {
	vec3 cc = XYZ(bbTemp);
	
	vec3 col = ChromaticityValue(cc);
	
	vec3 J = getJVector(NTSC_White, NTSC_Chromaticity, col);
	
	vec3 constrained = constrainRGB(J);
	
	vec3 n = normalizeRGB(constrained);
	
	return n;
}


uint   packSnorm2x12(vec2 v) { uvec2 d = uvec2(round(2047.5 + v*2047.5)); return d.x|(d.y<<12u); }
uint   packSnorm2x8( vec2 v) { uvec2 d = uvec2(round( 127.5 + v* 127.5)); return d.x|(d.y<< 8u); }
vec2 unpackSnorm2x8( uint d) { return vec2(uvec2(d,d>> 8)& 255u)/ 127.5 - 1.0; }
vec2 unpackSnorm2x12(uint d) { return vec2(uvec2(d,d>>12)&4095u)/2047.5 - 1.0; }


const float PI = 3.1415926535897932384626433832795;
const float PHI = 1.6180339887498948482045868343656;
float madfrac( float a,float b) { return a*b-floor(a*b); }
vec2  madfrac( vec2 a, float b) { return a*b-floor(a*b); }
float sf2id(vec3 p, float n) 
{
    float phi = min(atan(p.y, p.x), PI), cosTheta = p.z;
    
    float k  = max(2.0, floor( log(n * PI * sqrt(5.0) * (1.0 - cosTheta*cosTheta))/ log(PHI*PHI)));
    float Fk = pow(PHI, k)/sqrt(5.0);
    
    vec2 F = vec2( round(Fk), round(Fk * PHI) );

    vec2 ka = -2.0*F/n;
    vec2 kb = 2.0*PI*madfrac(F+1.0, PHI-1.0) - 2.0*PI*(PHI-1.0);    
    mat2 iB = mat2( ka.y, -ka.x, -kb.y, kb.x ) / (ka.y*kb.x - ka.x*kb.y);

    vec2 c = floor( iB * vec2(phi, cosTheta - (1.0-1.0/n)));
    float d = 8.0;
    float j = 0.0;
    for( int s=0; s<4; s++ ) 
    {
        vec2 uv = vec2( float(s-2*(s/2)), float(s/2) );
        
        float cosTheta = dot(ka, uv + c) + (1.0-1.0/n);
        
        cosTheta = clamp(cosTheta, -1.0, 1.0)*2.0 - cosTheta;
        float i = floor(n*0.5 - cosTheta*n*0.5);
        float phi = 2.0*PI*madfrac(i, PHI-1.0);
        cosTheta = 1.0 - (2.0*i + 1.0)/n;
        float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
        
        vec3 q = vec3( cos(phi)*sinTheta, sin(phi)*sinTheta, cosTheta);
        float squaredDistance = dot(q-p, q-p);
        if (squaredDistance < d) 
        {
            d = squaredDistance;
            j = i;
        }
    }
    return j;
}

vec3 id2sf( float i, float n) 
{
    float phi = 2.0*PI*madfrac(i,PHI);
    float zi = 1.0 - (2.0*i+1.0)/n;
    float sinTheta = sqrt( 1.0 - zi*zi);
    return vec3( cos(phi)*sinTheta, sin(phi)*sinTheta, zi);
}


uint fibonacci_8( in vec3 nor )
{
    return uint(sf2id(nor,exp2(8.0)));
}
vec3 i_fibonacci_8( uint data )
{
    return id2sf(float(data),exp2(8.0));
}
uint fibonacci_12( in vec3 nor )
{
    return uint(sf2id(nor,float(1<<12)));
}
vec3 i_fibonacci_12( uint data )
{
    return id2sf(float(data),float(1<<12));
}
uint fibonacci_14( in vec3 nor )
{
    return uint(sf2id(nor,float(1<<14)));
}
vec3 i_fibonacci_14( uint data )
{
    return id2sf(float(data),float(1<<14));
}
uint fibonacci_16( in vec3 nor )
{
    return uint(sf2id(nor,float(1<<16)));
}
vec3 i_fibonacci_16( uint data )
{
    return id2sf(float(data),float(1<<16));
}
vec2 msign( vec2 v )
{
    return vec2( (v.x>=0.0) ? 1.0 : -1.0, 
                 (v.y>=0.0) ? 1.0 : -1.0 );
}
uint octahedral_12( in vec3 nor )
{
    nor.xy /= ( abs( nor.x ) + abs( nor.y ) + abs( nor.z ) );
    nor.xy  = (nor.z >= 0.0) ? nor.xy : (1.0-abs(nor.yx))*msign(nor.xy);
    uvec2 d = uvec2(round(31.5 + nor.xy*31.5));  return d.x|(d.y<<6u);
}
vec3 i_octahedral_12( uint data )
{
    uvec2 iv = uvec2( data, data>>6u ) & 63u; vec2 v = vec2(iv)/31.5 - 1.0;
    vec3 nor = vec3(v, 1.0 - abs(v.x) - abs(v.y)); // Rune Stubbe's version,
    float t = max(-nor.z,0.0);                     // much faster than original
    nor.x += (nor.x>0.0)?-t:t;                     // implementation of this
    nor.y += (nor.y>0.0)?-t:t;                     // technique
    return normalize( nor );
}
uint octahedral_24( in vec3 nor )
{
    nor /= ( abs( nor.x ) + abs( nor.y ) + abs( nor.z ) );
    nor.xy = (nor.z >= 0.0) ? nor.xy : (1.0-abs(nor.yx))*msign(nor.xy);
    return packSnorm2x12(nor.xy);
}
vec3 i_octahedral_24( uint data )
{
    vec2 v = unpackSnorm2x12(data);
    vec3 nor = vec3(v, 1.0 - abs(v.x) - abs(v.y)); // Rune Stubbe's version,
    float t = max(-nor.z,0.0);                     // much faster than original
    nor.x += (nor.x>0.0)?-t:t;                     // implementation of this
    nor.y += (nor.y>0.0)?-t:t;                     // technique
    return normalize( nor );
}
uint spheremap_16( in vec3 nor )
{
    vec2 v = nor.xy*inversesqrt(2.0*nor.z+2.0);
    return packSnorm2x8(v);
}
vec3 i_spheremap_16( uint data )
{
    vec2 v = unpackSnorm2x8(data);
    float f = dot(v,v);
    return vec3( 2.0*v*sqrt(1.0-f), 1.0-2.0*f );
}
//random function borrowed from https://www.shadertoy.com/view/4tSXR1
vec2 rand(vec2 co){
	return vec2(
		//erratic trig functions my beloved
		//psuedorandom result, predictable but still varied enough
		fract(sin(dot(co.xy ,vec2(16.9198,78.233))) * 43858.5453),
		fract(cos(dot(co.yx ,vec2(8.6447,45.097))) * 43758.5453)
	);
}
vec3 rand(vec3 co){
	return vec3(
		//erratic trig functions my beloved
		//psuedorandom result, predictable but still varied enough
		fract(sin(dot(co.xyz ,vec3(34.9198,110.233,154.044))) * 43858.5453),
		fract(sin(dot(co.xyz ,vec3(16.9198,78.233,110.233))) * 43858.5453),
		fract(cos(dot(co.yxz ,vec3(8.6447,45.097,78.233))) * 43758.5453)
	);
}
float hash13n(vec3 p)
{
	p  = fract( p * vec3(5.3987, 5.4472, 6.9371) );
	p += dot( p.yzx, p.xyz + vec3(21.5351, 14.3137, 15.3247) );
	return fract( (p.x * p.y + p.z) * 95.4307 );
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

	const vec2 dim = vec2(1.0/16.0*3.1415926);
	vec2 pp = sp;
	vec2 np = mod2(pp, dim);

	vec3 col = vec3(0.0);

	float y = sin(sp.x);
	float d = min(abs(pp.x), abs(pp.y*y));

	float aa = 2.0/1000.0;

	col += 2.0*vec3(0.5, 0.5, 1.0)*exp(-2000.0*max(d-0.00025, 0.0));

	return 0.25*tanh(col);
}

mat3 yaw_pitch_roll(float yaw, float pitch, float roll)
{
	mat3 R = mat3(vec3(cos(yaw), sin(yaw), 0.0), vec3(-sin(yaw), cos(yaw), 0.0), vec3(0.0, 0.0, 1.0));
	mat3 S = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, cos(pitch), sin(pitch)), vec3(0.0, -sin(pitch), cos(pitch)));
	mat3 T = mat3(vec3(cos(roll), 0.0, sin(roll)), vec3(0.0, 1.0, 0.0), vec3(-sin(roll), 0.0, cos(roll)));

	return R * S * T;
}


vec3 stars_freq(vec3 ro, vec3 rd) {
	vec3 discretize = i_fibonacci_12(fibonacci_12(rd));
	float r = hash13n(discretize * 100);
	if (r > 0.05)
		return vec3(0.0);

	float temp = hash13n(discretize * 1000) * 5000.0 + 5000.0;
	float star_radius = hash13n(discretize * 100) * 0.003 + 0.002;
	float star_intensity = hash13n(discretize * 10);
	vec3 star_color = getRGB(temp);

	vec3 color = vec3(0.0);
	float d = distance(discretize, rd);

	// color += vec3(d);
	d = 1.0 - smoothstep(0.0, star_radius, d);

	color += star_color * d;

	d = distance(discretize, rd);
	d = 1.0 - smoothstep(0.0, star_radius * 2.5, d);
	color += star_color * d * 0.2;
	return color * star_intensity;
}

vec3 stars(vec3 ro, vec3 rd) {
	vec3 discretize = i_fibonacci_12(fibonacci_12(rd));
	vec3 color = vec3(0.0);

	for (int i = 0; i < 100; i += 1) {
		float yaw = hash13n(vec3(i * 3 + 0)) * 2 * PI;
		float pitch = hash13n(vec3(i * 3 + 1)) * PI;
		float roll = hash13n(vec3(i * 3 + 2)) * 2 * PI;

		mat3 R = yaw_pitch_roll(yaw, pitch, roll);

		color += stars_freq(ro, R * rd);
	}

	return color;
}

void main() {
	vec3 ray_world = world_ray();

	vec2 spher = toSpherical(ray_world).yz;
	vec3 col = empty_color;
	col += grid(vec3(0.0), ray_world, spher) * 0.1;
	col += stars(vec3(0.0), ray_world);
	fragColor = vec4(col, 1.0);
}
