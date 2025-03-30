#pragma once

#include "Common.hpp"
#include "Maths.hpp"

#include "SDL3/SDL.h"
#include "Random.hpp"
#include "Graphics.hpp"
#include "SDL3/SDL_gpu.h"

#include <vector>
#include <array>


struct Tile {
	enum class Kind: u8 {
		DEEP_OCEAN = 0,
		SHALLOW_OCEAN = 1,
		BEACH = 2,
		DESERT = 3,
		TUNDRA = 4,
		STEPPE = 5,
		FOREST = 6,
		RAIN_FOREST = 7,
		PEAK = 8,
		SNOW = 9,
		SNOW_PEAK = 10,
		ICE = 11,
		COUNT
	};

	Vector3f center;
	f32 height; // delta from the radius of the planet in km
	f32 year_temperature; // in celsius
	f32 heat_quantity;
	f32 base_pressure;
	f32 humidity;
	Vector3f macro_wind;
	Kind kind;

	size_t distanceToWater = SIZE_MAX;
	size_t nextTileToWater = SIZE_MAX;

	size_t na = SIZE_MAX;
	size_t nb = SIZE_MAX;
	size_t nc = SIZE_MAX;

	size_t plate_index = SIZE_MAX;
	f32 wind_step_to_moutain = 0.f;
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

		void upload(SDL_GPUDevice* gpu, std::vector<SDL_GPUFence*>& fences);
		bool create_pipeline(SDL_GPUDevice* gpu, SDL_GPUTextureFormat format);

		void release(SDL_GPUDevice* gpu);
	};

	struct Generation_Param {
		size_t octave = 5;
		f32 roughness = 0.3f;
		f32 lacunarity = 10.f;
		f32 water_level = 0.7f;
		f32 peak_level = 0.985f;
		size_t n_plates = 50;
		f32 plate_speed = 4.f;
		size_t plate_fail_smooth = 9;
		f32 plate_fail_smooth_factor = 0.65f;
		f32 average_temperature = 20.f;
	};

	struct Uniform {
		std::array<Vector4f, 64> palette;
		i32 overlay = 0;
	};

	Mesh mesh;
	WorldArrow vector_field;
	Uniform uniform;
	Common_Uniform common_uniform;
	std::vector<Tile> tiles;
	std::vector<Plate> plates;

	f32 min_height = +FLT_MAX;
	f32 max_height = -FLT_MAX;
	f32 min_year_temp = +FLT_MAX;
	f32 max_year_temp = -FLT_MAX;

	xorshift128p seed;

	enum class Overlay_Render {
		None,
		Height,
		WaterDistance,
		TectonicPlates,
		Temperature,
		Pressure,
		MacroWind,
		WindStepToMoutain,
		Humidity,
		HeatQuantity,
		Count
	} overlay_render = Overlay_Render::None;

	Vector3f position = { 0, 0, 0 };
	Quaternionf orientation = { 0, 0, 0, 1 };

	size_t order = 6;
	Generation_Param generation_param;

	f32 time = 0.0f;
	f32 time_day = 0.0f;
	f32 time_year = 0.0f;

	f32 avg_temperature = 20.f; // in celsius
	f32 radius = 10.0f; // in 1000s of km
	f32 day_period = 300.0f; // in seconds
	f32 year_period = 3000.0f; // in seconds
	f32 orbit_ecentricity = 0.1f; // unitless
	f32 orbit_inclination = 7.1f; // in deg
	f32 axial_tilt = 22.5f; // in deg

	// f32 min_temp_desert = 31.f;
	// f32 max_temp_tundra = 29.5f;
	// f32 humidity_desert = 0.15f;
	// f32 humidity_steppe = 0.2f;
	// f32 humidity_rainforest = 0.55f;
	// f32 snow_peak_factor = 1.5f;
	// f32 max_snow_temp = 25.f;
	f32 min_temp_desert = 30.f;
	f32 max_temp_tundra = 29.75f;
	f32 humidity_desert = 0.19f;
	f32 humidity_steppe = 0.195f;
	f32 humidity_rainforest = 0.2f;
	f32 snow_peak_factor = 0.8f;
	f32 max_ice_temp = 25.f;
	f32 max_snow_temp = 28.f;

	bool render_vector_field = false;

	Planet();

	void release(SDL_GPUDevice* gpu);
	void create_pipeline(SDL_GPUDevice* gpu, SDL_GPUTextureFormat format);

	void upload(SDL_GPUDevice* gpu, std::vector<SDL_GPUFence*>& fences);
	void update(f32 dt);
	void render(SDL_GPURenderPass* pass, SDL_GPUCommandBuffer* command);
	void imgui(SDL_GPUDevice* gpu);

	void generate_icosphere(SDL_GPUDevice* gpu, size_t order);

	void generate_from_mesh(const Generation_Param& param);

	void fill_height(size_t octave, f32 roughness, f32 lacunarity);
	void fill_year_temperature();
	void fill_base_pressure();
	void fill_macro_wind();
	void fill_wind_step_to_moutain();
	void find_water(f32 water_level, f32 peak_level);
	void fill_humidity();
	void grow_plates(size_t n_plates, f32 plate_speed, size_t fail_smooth, f32 fail_smooth_factor);
	void categorize_tiles();
	void final_categorize_tiles();

	Vector3f get_rotation_axis();
	Vector3f get_zero_longitude_axis();
};
