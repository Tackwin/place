#include "Random.hpp"

u64 next(xorshift128p& state) {
	u64 s1 = state.s[0];
	u64 s0 = state.s[1];
	u64 result = s0 + s1;
	state.s[0] = s0;
	s1 ^= s1 << 23;
	state.s[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5);
	return result;
}
f32 uniform(xorshift128p& state) {
	return (f32)next(state) / (f32)SIZE_MAX;
}
u64 uniformi(xorshift128p& state, u64 min, u64 max) {
	return min + (next(state) % (max - min));
}

