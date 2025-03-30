#pragma once
using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;
using i32 = signed int;
using i64 = signed long long;
using f32 = float;
using f64 = double;
using usz = size_t;

#ifndef SIZE_MAX
constexpr size_t SIZE_MAX = (size_t)-1;
#endif

// defer implementation
struct defer_dummy {};
template <class F> struct deferrer { F f; ~deferrer() { f(); } };
template <class F> deferrer<F> operator*(defer_dummy, F f) { return {f}; }

#define DEFER_(LINE) _defer##LINE
#define DEFER(LINE) DEFER_(LINE)
#define defer auto DEFER(__COUNTER__) = defer_dummy{} *[&]()

constexpr f64 PId = 3.14159265358979323846;
constexpr f32 PIf = 3.14159265358979323846f;
constexpr f32 RAD_DEGf = 180.0f / PIf;
constexpr f32 DEG_RADf = PIf / 180.0f;
