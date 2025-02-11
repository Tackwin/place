#pragma once
#include "Common.hpp"

extern f32 perlin(f32 x, f32 y, f32 z);
extern f32 fractal_perlin(f32 x, f32 y, f32 z, size_t octaves, f32 roughness, f32 lacunarity);