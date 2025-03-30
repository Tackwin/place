#include "Maths.hpp"

Vector2f normalize(Vector2f v) {
	f32 l = sqrtf(v.x * v.x + v.y * v.y);
	return {
		v.x / l,
		v.y / l
	};
}
f32 dot(Vector2f a, Vector2f b) {
	return a.x * b.x + a.y * b.y;
}
f32 length(Vector2f v) {
	return sqrtf(v.x * v.x + v.y * v.y);
}

Vector3f::Vector3f(const Vector4f& v) {
	x = v.x;
	y = v.y;
	z = v.z;
}

Vector3f& Vector3f::operator*=(f32 s) {
	x *= s;
	y *= s;
	z *= s;
	return *this;
}

Vector3f operator*(Vector3f v, f32 s) {
	return {
		v.x * s,
		v.y * s,
		v.z * s
	};
}
Vector3f operator*(f32 s, Vector3f v) {
	return {
		v.x * s,
		v.y * s,
		v.z * s
	};
}
Vector3f operator/(Vector3f v, f32 s) {
	return {
		v.x / s,
		v.y / s,
		v.z / s
	};
}
Vector3f operator-(Vector3f a, Vector3f b) {
	return {
		a.x - b.x,
		a.y - b.y,
		a.z - b.z
	};
}
Vector3f operator+(Vector3f a, Vector3f b) {
	return {
		a.x + b.x,
		a.y + b.y,
		a.z + b.z
	};
}

Vector3f cross(Vector3f a, Vector3f b) {
	return {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

f32 dot(Vector3f a, Vector3f b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

f32 length(Vector3f v) {
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vector3f normalize(Vector3f v) {
	f32 l = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return {
		v.x / l,
		v.y / l,
		v.z / l
	};
}

f32 angle(Vector3f a, Vector3f b) {
	f32 c = dot(a, b);
	f32 s = length(cross(a, b));

	if (c < -1)
		c = -1;
	if (s < -1)
		s = -1;
	if (c > 1)
		c = 1;
	if (s > 1)
		s = 1;

	return atan2f(s, c);
}


Quaternionf Quaternionf::axis_angle(Vector3f axis, f32 angle) {
	f32 s = sinf(angle / 2);
	return {
		axis.x * s,
		axis.y * s,
		axis.z * s,
		cosf(angle / 2)
	};
}

Quaternionf Quaternionf::from_unit_vectors(Vector3f a, Vector3f b) {
	if (a.x == -b.x && a.y == -b.y && a.z == -b.z) {
		Vector3f axis = normalize(cross({1, 0, 0}, a));
		return axis_angle(axis, 3.1415926);
	}
	Vector3f half = normalize(a + b);
	return { cross(a, half), dot(a, half) };
}

std::optional<Vector3f> intersect_sphere_ray(Vector3f o, f32 r, Vector3f p, Vector3f d) {
	Vector3f op = o - p;
	f32 b = dot(op, d);
	f32 det = b * b - dot(op, op) + r * r;
	if (det < 0) {
		return std::nullopt;
	}
	det = sqrtf(det);
	f32 t = b - det;
	if (t > 0) {
		return p + d * t;
	}
	t = b + det;
	if (t > 0) {
		return p + d * t;
	}
	return std::nullopt;
}

Matrix4f Matrix4f::by_cols(Vector4f a, Vector4f b, Vector4f c, Vector4f d) {
	Matrix4f mm;
	f32* m = mm.m;
	m[0] = a.x;
	m[1] = b.x;
	m[2] = c.x;
	m[3] = d.x;
	m[4] = a.y;
	m[5] = b.y;
	m[6] = c.y;
	m[7] = d.y;
	m[8] = a.z;
	m[9] = b.z;
	m[10] = c.z;
	m[11] = d.z;
	m[12] = a.w;
	m[13] = b.w;
	m[14] = c.w;
	m[15] = d.w;
	return mm;
}

Matrix4f perspective(f32 fov, f32 aspect, f32 near, f32 far) {
	f32 f = 1.0f / tanf((fov * DEG_RADf) / 2.0f);
	Matrix4f m = {
		f / aspect, 0.0f, 0.0f, 0.0f,
		0.0f, f, 0.0f, 0.0f,
		0.0f, 0.0f, -(far + near) / (far - near), -1.0f,
		0.0f, 0.0f, -2 * (far * near) / (far - near), 0.0f
	};
	return m;
}
Matrix4f identity() {
	Matrix4f m;
	memset(m.m, 0, sizeof(m));
	m.m[0] = 1.0f;
	m.m[5] = 1.0f;
	m.m[10] = 1.0f;
	m.m[15] = 1.0f;
	return m;
}

Matrix4f translation(Vector3f v) {
	Matrix4f m = identity();
	m.m[12] = v.x;
	m.m[13] = v.y;
	m.m[14] = v.z;
	return m;
}

Matrix4f inverse(const Matrix4f& mm);
Matrix4f lookAt(Vector3f eye, Vector3f target, Vector3f up) {
	Matrix4f m;
	memset(m.m, 0, sizeof(m));
	Vector3f z = normalize(eye - target);
	Vector3f x = normalize(cross(up, z));
	Vector3f y = cross(z, x);
	m.m[0]  = x.x;
	m.m[1]  = y.x;
	m.m[2]  = z.x;
	m.m[3]  = 0;
	m.m[4]  = x.y;
	m.m[5]  = y.y;
	m.m[6]  = z.y;
	m.m[7]  = 0;
	m.m[8]  = x.z;
	m.m[9]  = y.z;
	m.m[10] = z.z;
	m.m[11] = 0;
	m.m[12] = -dot(eye, x);
	m.m[13] = -dot(eye, y);
	m.m[14] = -dot(eye, z);
	m.m[15] = 1.0f;
	return m;
}

Matrix4f operator*(const Matrix4f& A, const Matrix4f& B) {
	Matrix4f C;
	for (size_t i = 0; i < 4; i += 1)
	for (size_t j = 0; j < 4; j += 1)
	for (size_t k = 0; k < 4; k += 1) {
		C.m[i * 4 + j] += A.m[k * 4 + j] * B.m[i * 4 + k];
	}
	return C;
}

Vector4f operator*(const Matrix4f& A, const Vector4f& b) {
	Vector4f C;
	f32* cv = &C.x;
	const f32* bv = &b.x;

	for (size_t i = 0; i < 4; i += 1)
	for (size_t j = 0; j < 4; j += 1) {
		cv[i] += A.m[i * 4 + j] * bv[j];
	}
	return C;
}

Matrix4f inverse(const Matrix4f& mm) {
	Matrix4f minv;
	f32* inv = minv.m;
	const f32* m = mm.m;

	inv[0] = m[5]  * m[10] * m[15] -
				m[5]  * m[11] * m[14] -
				m[9]  * m[6]  * m[15] +
				m[9]  * m[7]  * m[14] +
				m[13] * m[6]  * m[11] -
				m[13] * m[7]  * m[10];

	inv[4] = -m[4]  * m[10] * m[15] +
				m[4]  * m[11] * m[14] +
				m[8]  * m[6]  * m[15] -
				m[8]  * m[7]  * m[14] -
				m[12] * m[6]  * m[11] +
				m[12] * m[7]  * m[10];

	inv[8] = m[4]  * m[9] * m[15] -
				m[4]  * m[11] * m[13] -
				m[8]  * m[5] * m[15] +
				m[8]  * m[7] * m[13] +
				m[12] * m[5] * m[11] -
				m[12] * m[7] * m[9];

	inv[12] = -m[4]  * m[9] * m[14] +
				m[4]  * m[10] * m[13] +
				m[8]  * m[5] * m[14] -
				m[8]  * m[6] * m[13] -
				m[12] * m[5] * m[10] +
				m[12] * m[6] * m[9];

	inv[1] = -m[1]  * m[10] * m[15] +
				m[1]  * m[11] * m[14] +
				m[9]  * m[2] * m[15] -
				m[9]  * m[3] * m[14] -
				m[13] * m[2] * m[11] +
				m[13] * m[3] * m[10];

	inv[5] = m[0]  * m[10] * m[15] -
				m[0]  * m[11] * m[14] -
				m[8]  * m[2] * m[15] +
				m[8]  * m[3] * m[14] +
				m[12] * m[2] * m[11] -
				m[12] * m[3] * m[10];

	inv[9] = -m[0]  * m[9] * m[15] +
				m[0]  * m[11] * m[13] +
				m[8]  * m[1] * m[15] -
				m[8]  * m[3] * m[13] -
				m[12] * m[1] * m[11] +
				m[12] * m[3] * m[9];

	inv[13] = m[0]  * m[9] * m[14] -
				m[0]  * m[10] * m[13] -
				m[8]  * m[2] * m[13] +
				m[8]  * m[1] * m[14] +
				m[12] * m[1] * m[10] -
				m[12] * m[2] * m[9];

	inv[2] = m[1]  * m[6] * m[15] -
				m[1]  * m[7] * m[14] -
				m[5]  * m[2] * m[15] +
				m[5]  * m[3] * m[14] +
				m[13] * m[2] * m[7] -
				m[13] * m[3] * m[6];

	inv[6] = -m[0]  * m[6] * m[15] +
				m[0]  * m[7] * m[14] +
				m[4]  * m[2] * m[15] -
				m[4]  * m[3] * m[14] -
				m[12] * m[2] * m[7] +
				m[12] * m[3] * m[6];

	inv[10] = m[0]  * m[5] * m[15] -
				m[0]  * m[7] * m[13] -
				m[4]  * m[1] * m[15] +
				m[4]  * m[3] * m[13] +
				m[12] * m[1] * m[7] -
				m[12] * m[3] * m[5];

	inv[14] = -m[0]  * m[5] * m[14] +
				m[0]  * m[6] * m[13] +
				m[4]  * m[1] * m[14] -
				m[4]  * m[2] * m[13] -
				m[12] * m[1] * m[6] +
				m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] +
				m[1] * m[7] * m[10] +
				m[5] * m[2] * m[11] -
				m[5] * m[3] * m[10] -
				m[9] * m[2] * m[7] +
				m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] -
				m[0] * m[7] * m[10] -
				m[4] * m[2] * m[11] +
				m[4] * m[3] * m[10] +
				m[8] * m[2] * m[7] -
				m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] +
				m[0] * m[7] * m[9] +
				m[4] * m[1] * m[11] -
				m[4] * m[3] * m[9] -
				m[8] * m[1] * m[7] +
				m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] -
				m[0] * m[6] * m[9] -
				m[4] * m[1] * m[10] +
				m[4] * m[2] * m[9] +
				m[8] * m[1] * m[6] -
				m[8] * m[2] * m[5];

	f32 det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * m[12];
	if (det == 0)
		return {};
	det = 1.0f / det;

	for (i32 i = 0; i < 16; i++)
		inv[i] = inv[i] * det;

	return minv;
}

Vector3f operator*(Quaternionf q, Vector3f v) {
	Vector3f u = {q.x, q.y, q.z};
	f32 s = q.w;

	return 2 * dot(u, v) * u
		+ (s * s - dot(u, u)) * v
		+ 2 * s * cross(u, v);
}

Matrix4f to_rotation_matrix(const Quaternionf& q) {
	Vector3f x = q * Vector3f(1, 0, 0);
	Vector3f y = q * Vector3f(0, 1, 0);
	Vector3f z = q * Vector3f(0, 0, 1);
	return Matrix4f::by_cols(
		{ x.x, y.x, z.x, 0.f },
		{ x.y, y.y, z.y, 0.f },
		{ x.z, y.z, z.z, 0.f },
		{ 0.f, 0.f, 0.f, 1.f }
	);
}

Quaternionf operator*(const Quaternionf& a, const Quaternionf& b)
{
	return {
		a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,  // i
		a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,  // j
		a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,  // k
		a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,  // 1
	};
}

Quaternionf::operator Matrix4f()
{
	f32 m[9];
	f32 s = 2.0f / (x * x + y * y + z * z + w * w);
	m[0] = 1 - s * (y * y + z * z);
	m[1] = s * (x * y - z * w);
	m[2] = s * (x * z + y * w);
	m[3] = s * (x * y + z * w);
	m[4] = 1 - s * (x * x + z * z);
	m[5] = s * (y * z - x * w);
	m[6] = s * (x * z - y * w);
	m[7] = s * (y * z + x * w);
	m[8] = 1 - s * (x * x + y * y);
	Matrix4f mm;
	mm.m[0] = m[0];
	mm.m[1] = m[1];
	mm.m[2] = m[2];
	mm.m[3] = 0;
	mm.m[4] = m[3];
	mm.m[5] = m[4];
	mm.m[6] = m[5];
	mm.m[7] = 0;
	mm.m[8] = m[6];
	mm.m[9] = m[7];
	mm.m[10] = m[8];
	mm.m[11] = 0;
	mm.m[12] = 0;
	mm.m[13] = 0;
	mm.m[14] = 0;
	mm.m[15] = 1;
	return mm;
}
