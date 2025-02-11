#pragma once
#include "Common.hpp"
#include <math.h>
#include <optional>

struct Vector4f;

struct Vector2f {
	f32 x = 0;
	f32 y = 0;
};
extern Vector2f normalize(Vector2f v);
extern f32 dot(Vector2f a, Vector2f b);
extern f32 length(Vector2f v);

struct Vector3f {
	f32 x = 0;
	f32 y = 0;
	f32 z = 0;

	Vector3f() : x(0), y(0), z(0) {}
	Vector3f(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}
	Vector3f(const Vector4f& v);

	Vector3f& operator*=(f32 s);
};
struct Vector4f {
	f32 x = 0;
	f32 y = 0;
	f32 z = 0;
	f32 w = 0;

	Vector4f() : x(0), y(0), z(0), w(0) {}
	Vector4f(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
	Vector4f(Vector3f v, f32 w) : x(v.x), y(v.y), z(v.z), w(w) {}
};
Vector3f operator*(Vector3f v, f32 s);
Vector3f operator*(f32 s, Vector3f v);
Vector3f operator/(Vector3f v, f32 s);
Vector3f operator-(Vector3f a, Vector3f b);
Vector3f operator+(Vector3f a, Vector3f b);
Vector3f cross(Vector3f a, Vector3f b);
f32 dot(Vector3f a, Vector3f b);
f32 length(Vector3f v);
Vector3f normalize(Vector3f v);

std::optional<Vector3f> intersect_sphere_ray(Vector3f o, f32 r, Vector3f p, Vector3f d);
struct Matrix4f {
	f32 m[16] = {0};

	static Matrix4f by_cols(Vector4f a, Vector4f b, Vector4f c, Vector4f d);
};
struct Quaternionf {
	f32 x = 0;
	f32 y = 0;
	f32 z = 0;
	f32 w = 0;

	Quaternionf() : x(0), y(0), z(0), w(0) {}
	Quaternionf(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
	Quaternionf(Vector3f vector, f32 scalar) {
		x = vector.x;
		y = vector.y;
		z = vector.z;
		w = scalar;
	}

	static Quaternionf axis_angle(Vector3f axis, f32 angle);
	static Quaternionf from_unit_vectors(Vector3f a, Vector3f b);
	
	operator Matrix4f();
};
Quaternionf operator*(const Quaternionf& a, const Quaternionf& b);
Matrix4f perspective(f32 fov, f32 aspect, f32 near, f32 far);
Matrix4f identity();
Matrix4f translation(Vector3f v);
Matrix4f inverse(const Matrix4f& mm);
Matrix4f lookAt(Vector3f eye, Vector3f target, Vector3f up = { 0, 0, 1 });
Matrix4f operator*(const Matrix4f& A, const Matrix4f& B);
Vector4f operator*(const Matrix4f& A, const Vector4f& b);
Matrix4f inverse(const Matrix4f& mm);
Vector3f operator*(Quaternionf q, Vector3f v);
Matrix4f to_rotation_matrix(const Quaternionf& q);
