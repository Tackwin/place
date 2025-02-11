#pragma once

#include "Common.hpp"
#include "Maths.hpp"
#include "SDL3/SDL.h"
#include "Random.hpp"
#include "Graphics.hpp"

#include <vector>
#include <array>


struct Tile {
	enum class Kind: u8 {
		DEEP_OCEAN = 0,
		SHALLOW_OCEAN = 1,
		BEACH = 2,
		FOREST = 3,
		PEAK = 4,
		COUNT
	};

	Vector3f center;
	float height;
	Kind kind;

	size_t distanceToWater = SIZE_MAX;
	size_t nextTileToWater = SIZE_MAX;

	size_t na = SIZE_MAX;
	size_t nb = SIZE_MAX;
	size_t nc = SIZE_MAX;

	size_t plate_index = SIZE_MAX;
};

struct Plate {
	f32 angle;
	f32 speed;
};

struct Planet {
	struct Mesh {
		struct Vertex {
			Vector3f position;
			Vector3f normal;
			f32 scalar;
			u32 palette_index;
			u32 triangle_index;
		};

		std::vector<Vertex> vertices;

		SDL_GPUBuffer* gpu_vertex_buffer = nullptr;
		SDL_GPUTransferBuffer* gpu_transfer_buffer = nullptr;
		Matrix4f local = identity();

		SDL_GPUGraphicsPipeline* pipeline = nullptr;

		SDL_GPUFence* upload(SDL_GPUDevice* gpu);
		bool create_pipeline(SDL_GPUDevice* gpu);

		void release(SDL_GPUDevice* gpu);
	};

	struct Generation_Param {
		size_t octave = 5;
		f32 roughness = 0.3f;
		f32 lacunarity = 10.f;
		f32 water_level = 0.7f;
		f32 peak_level = 0.98f;
		size_t n_plates = 50;
		f32 plate_speed = 2.5f;
		size_t plate_fail_smooth = 7;
		f32 plate_fail_smooth_factor = 0.8f;
	};

	struct Uniform {
		std::array<Vector4f, 64> palette;
		i32 overlay = 0;
	};

	Mesh mesh;
	Uniform uniform;
	Common_Uniform common_uniform;
	std::vector<Tile> tiles;
	std::vector<Plate> plates;
	
	xorshift128p seed;

	enum class Overlay_Render {
		None,
		Height,
		WaterDistance,
		TectonicPlates,
		Count
	} overlay_render = Overlay_Render::None;

	Vector3f position = { 0, 0, 0 };
	Quaternionf orientation = { 0, 0, 0, 1 };

	size_t order = 6;
	Generation_Param generation_param;

	f32 time = 0.0f;
	f32 time_day = 0.0f;
	f32 time_year = 0.0f;

	f32 radius = 10.0f;
	f32 day_period = 300.0f;
	f32 year_period = 3000.0f;
	f32 orbit_ecentricity = 0.1f;
	f32 orbit_inclination = 7.1f;
	f32 axial_tilt = 22.5f;

	Planet();

	void release(SDL_GPUDevice* gpu);

	void update(f32 dt);
	void render(SDL_GPURenderPass* pass, SDL_GPUCommandBuffer* command);
	void imgui(SDL_GPUDevice* gpu);

	void generate_icosphere(SDL_GPUDevice* gpu, size_t order);

	void generate_from_mesh(const Generation_Param& param);

	void fill_height(size_t octave, f32 roughness, f32 lacunarity);
	void find_water(f32 water_level, f32 peak_level);
	void grow_plates(size_t n_plates, f32 plate_speed, size_t fail_smooth, f32 fail_smooth_factor);
	void categorize_tiles();

};
