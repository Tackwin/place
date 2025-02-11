#pragma once

#include "Common.hpp"

struct xorshift128p {
	u64 s[2];
};

extern u64 next(xorshift128p& state);
extern f32 uniform(xorshift128p& state);
extern u64 uniformi(xorshift128p& state, u64 min, u64 max);
