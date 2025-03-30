#include "Planet.hpp"

#include "Common.hpp"
#include "Graphics.hpp"
#include "Maths.hpp"
#include "Noise.hpp"
#include "SDL3/SDL_gpu.h"
#include "imgui/imgui.h"

#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <vector>

Planet::Planet() {
	seed.s[0] = 1234;
	seed.s[1] = 5678;

	uniform.palette[(u8)Tile::Kind::DEEP_OCEAN]    = {
		0x00 / 255.f, 0x1A / 255.f, 0x33 / 255.f, 0.f
	};
	uniform.palette[(u8)Tile::Kind::SHALLOW_OCEAN] = {
		0x00 / 255.f, 0x66 / 255.f, 0xCC / 255.f, 0.f
	};
	uniform.palette[(u8)Tile::Kind::BEACH]         = {
		0xA0 / 255.f, 0x90 / 255.f, 0x77 / 255.f, 0.f
	};
	uniform.palette[(u8)Tile::Kind::FOREST]        = {
		0x33 / 255.f, 0x77 / 255.f, 0x55 / 255.f, 0.f
	};
	uniform.palette[(u8)Tile::Kind::PEAK]          = {
		0x96 / 255.f, 0x99 / 255.f, 0x97 / 255.f, 0.f
	};
	uniform.palette[(u8)Tile::Kind::SNOW]          = {
		0xFF / 255.f, 0xFF / 255.f, 0xFF / 255.f, 0.f
	};
	uniform.palette[(u8)Tile::Kind::SNOW_PEAK]          = {
		0xE4 / 255.f, 0xE4 / 255.f, 0xE4 / 255.f, 0.f
	};
	uniform.palette[(u8)Tile::Kind::RAIN_FOREST]          = {
		0x15 / 255.f, 0x3C / 255.f, 0x28 / 255.f, 0.f
	};
	uniform.palette[(u8)Tile::Kind::TUNDRA]          = {
		0x63 / 255.f, 0x72 / 255.f, 0x50 / 255.f, 0.f
	};
	uniform.palette[(u8)Tile::Kind::STEPPE]          = {
		0x84 / 255.f, 0x8D / 255.f, 0x3E / 255.f, 0.f
	};
	uniform.palette[(u8)Tile::Kind::DESERT]          = {
		0xC7 / 255.f, 0xBB / 255.f, 0x55 / 255.f, 0.f
	};
	uniform.palette[(u8)Tile::Kind::ICE]          = {
		0xC5 / 255.f, 0xDB / 255.f, 0xF1 / 255.f, 0.f
	};
}

void Planet::create_pipeline(SDL_GPUDevice* gpu, SDL_GPUTextureFormat format) {
	mesh.create_pipeline(gpu, format);
	vector_field.create_pipeline(gpu, format);
}

void Planet::release(SDL_GPUDevice* gpu) {
	mesh.release(gpu);
	vector_field.release(gpu);
}

void Planet::Mesh::release(SDL_GPUDevice* gpu) {
	if (gpu_vertex_buffer) {
		SDL_ReleaseGPUBuffer(gpu, gpu_vertex_buffer);
	}
	if (gpu_transfer_buffer) {
		SDL_ReleaseGPUTransferBuffer(gpu, gpu_transfer_buffer);
	}
	if (pipeline) {
		SDL_ReleaseGPUGraphicsPipeline(gpu, pipeline);
	}
}

void Planet::generate_icosphere(SDL_GPUDevice* gpu, size_t order) {
	float f = (1 + sqrtf(5)) / 2;
	std::vector<Vector3f> positions;
	positions.push_back(normalize({-1, +f, +0}));
	positions.push_back(normalize({+1, +f, +0}));
	positions.push_back(normalize({-1, -f, +0}));
	positions.push_back(normalize({+1, -f, +0}));
	positions.push_back(normalize({+0, -1, +f}));
	positions.push_back(normalize({+0, +1, +f}));
	positions.push_back(normalize({+0, -1, -f}));
	positions.push_back(normalize({+0, +1, -f}));
	positions.push_back(normalize({+f, +0, -1}));
	positions.push_back(normalize({+f, +0, +1}));
	positions.push_back(normalize({-f, +0, -1}));
	positions.push_back(normalize({-f, +0, +1}));

	size_t v = 12;
	std::vector<size_t> indices{
		0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11,
		11, 10, 2, 5, 11, 4, 1, 5, 9, 7, 1, 8, 10, 7, 6,
		3, 9, 4, 3, 4, 2, 3, 2, 6, 3, 6, 8, 3, 8, 9,
		9, 8, 1, 4, 9, 5, 2, 4, 11, 6, 2, 10, 8, 6, 7
	};

	std::unordered_map<size_t, size_t> cache;

	auto add_mid_point = [&] (size_t a, size_t b) {
		size_t key = (a + b) * (a + b + 1) / 2 + std::min(a, b);
		if (auto it = cache.find(key); it != cache.end()) {
			return it->second;
		}

		cache[key] = v;
		positions.push_back(normalize((positions[a] + positions[b]) * 0.5f));
		return v++;
	};

	auto indices_prev = indices;
	for (size_t i = 0; i < order; i++) {
		indices.clear();
		indices.reserve(indices_prev.size() * 4);

		for (size_t j = 0; j < indices_prev.size(); j += 3) {
			size_t v1 = indices_prev[j + 0];
			size_t v2 = indices_prev[j + 1];
			size_t v3 = indices_prev[j + 2];

			size_t a = add_mid_point(v1, v2);
			size_t b = add_mid_point(v2, v3);
			size_t c = add_mid_point(v3, v1);
			size_t t = j * 4 / 3;
			indices.push_back(v1);
			indices.push_back(a);
			indices.push_back(c);
			indices.push_back(v2);
			indices.push_back(b);
			indices.push_back(a);
			indices.push_back(v3);
			indices.push_back(c);
			indices.push_back(b);
			indices.push_back(a);
			indices.push_back(b);
			indices.push_back(c);
		}

		indices_prev = indices;
	}


	auto rand_vector = [] (size_t i) -> Vector3f {
		return normalize({
			(f32)(rand() / (f32)RAND_MAX) * 2 - 1,
			(f32)(rand() / (f32)RAND_MAX) * 2 - 1,
			(f32)(rand() / (f32)RAND_MAX) * 2 - 1
		});
	};

	for (size_t i = 0; i < positions.size(); i += 1) {
		positions[i] = normalize(positions[i]);
		positions[i] = positions[i] + rand_vector(i) * 0.15f / powf(2, order);
		positions[i] = normalize(positions[i]);
	}

	mesh.vertices.resize(indices.size());
	for (size_t i = 0; i < indices.size(); i++) {
		mesh.vertices[i].position = positions[indices[i]];
	}

	tiles.resize(mesh.vertices.size() / 3);
	std::unordered_map<size_t, size_t> edge_to_face;
	auto edge_to_key = [] (size_t a, size_t b) -> size_t {
		return (a + b) * (a + b + 1) / 2 + std::min(a, b);
	};
	for (size_t i = 0; i < indices.size(); i += 3) {
		size_t a = indices[i + 0];
		size_t b = indices[i + 1];
		size_t c = indices[i + 2];

		size_t ea = edge_to_key(a, b);
		size_t eb = edge_to_key(b, c);
		size_t ec = edge_to_key(c, a);

		if (auto it = edge_to_face.find(ea); it != std::end(edge_to_face)) {
			tiles[i / 3].na = it->second;
			if (edge_to_key(indices[it->second * 3 + 0], indices[it->second * 3 + 1]) == ea) {
				tiles[it->second].na = i / 3;
			}
			else if (edge_to_key(indices[it->second * 3 + 1], indices[it->second * 3 + 2]) == ea) {
				tiles[it->second].nb = i / 3;
			}
			else {
				tiles[it->second].nc = i / 3;
			}
		} else {
			edge_to_face[ea] = i / 3;
		}

		if (auto it = edge_to_face.find(eb); it != std::end(edge_to_face)) {
			tiles[i / 3].nb = it->second;
			if (edge_to_key(indices[it->second * 3 + 0], indices[it->second * 3 + 1]) == eb) {
				tiles[it->second].na = i / 3;
			}
			else if (edge_to_key(indices[it->second * 3 + 1], indices[it->second * 3 + 2]) == eb) {
				tiles[it->second].nb = i / 3;
			}
			else {
				tiles[it->second].nc = i / 3;
			}
		} else {
			edge_to_face[eb] = i / 3;
		}

		if (auto it = edge_to_face.find(ec); it != std::end(edge_to_face)) {
			tiles[i / 3].nc = it->second;
			if (edge_to_key(indices[it->second * 3 + 0], indices[it->second * 3 + 1]) == ec) {
				tiles[it->second].na = i / 3;
			}
			else if (edge_to_key(indices[it->second * 3 + 1], indices[it->second * 3 + 2]) == ec) {
				tiles[it->second].nb = i / 3;
			}
			else {
				tiles[it->second].nc = i / 3;
			}
		} else {
			edge_to_face[ec] = i / 3;
		}
	}

	for (size_t i = 0; i < mesh.vertices.size(); i += 3) {
		Vector3f a = mesh.vertices[i + 0].position;
		Vector3f b = mesh.vertices[i + 1].position;
		Vector3f c = mesh.vertices[i + 2].position;

		Vector3f center = (a + b + c) * (1 / 3.0f);

		Vector3f normal = cross(c - a, b - a);
		mesh.vertices[i + 0].normal = normal;
		mesh.vertices[i + 1].normal = normal;
		mesh.vertices[i + 2].normal = normal;

		mesh.vertices[i + 0].position = center + (a - center);
		mesh.vertices[i + 1].position = center + (b - center);
		mesh.vertices[i + 2].position = center + (c - center);

		mesh.vertices[i + 0].triangle_index = 0;
		mesh.vertices[i + 1].triangle_index = 1;
		mesh.vertices[i + 2].triangle_index = 2;

		tiles[i / 3].center = center;
	}

	if (mesh.gpu_vertex_buffer) {
		SDL_ReleaseGPUBuffer(gpu, mesh.gpu_vertex_buffer);
	}
	mesh.gpu_vertex_buffer = SDL_CreateGPUBuffer(
		gpu,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
			.size = (u32)(mesh.vertices.size() * sizeof(*mesh.vertices.data()))
		}
	);

	if (mesh.gpu_transfer_buffer) {
		SDL_ReleaseGPUTransferBuffer(gpu, mesh.gpu_transfer_buffer);
	}
	mesh.gpu_transfer_buffer = SDL_CreateGPUTransferBuffer(
		gpu, &(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = (u32)(mesh.vertices.size() * sizeof(*mesh.vertices.data()))
		}
	);
}

void Planet::generate_from_mesh(const Planet::Generation_Param& param) {
	generation_param = param;

	fill_height(param.octave, param.roughness, param.lacunarity);
	grow_plates(
		param.n_plates, param.plate_speed, param.plate_fail_smooth, param.plate_fail_smooth_factor
	);
	find_water(param.water_level, param.peak_level);
	categorize_tiles();
	fill_year_temperature();
	fill_base_pressure();
	fill_macro_wind();
	fill_wind_step_to_moutain();
	fill_humidity();
}

void Planet::fill_height(size_t octave, f32 roughness, f32 lacunarity) {
	for (size_t i = 0; i < mesh.vertices.size(); i += 3) {
		Vector3f a = mesh.vertices[i + 0].position;
		Vector3f b = mesh.vertices[i + 1].position;
		Vector3f c = mesh.vertices[i + 2].position;

		Vector3f center = (a + b + c) * (1 / 3.0f);
		center = center * 0.5f + Vector3f(0.5f, 0.5f, 0.5f);
		f32 height = fractal_perlin(center.x, center.y, center.z, octave, roughness, lacunarity);
		height *= 10;

		tiles[i / 3].height = height;
		min_height = std::min(min_height, height);
		max_height = std::max(max_height, height);
	}
}

void Planet::fill_year_temperature() {
	auto p2 = [] (f32 b) -> f32 {
		return (3 * b * b - 1) / 2;
	};
	auto p4 = [] (f32 b) -> f32 {
		return (35 * b * b * b * b - 30 * b * b + 3) / 8;
	};
	auto p6 = [] (f32 b) -> f32 {
		return (231 * b * b * b * b * b * b - 315 * b * b * b * b + 105 * b * b - 5) / 16;
	};

	auto sig = [&] (f32 y, f32 b) -> f32 {
		f32 ret = 1.0f;
		ret -= 5 * p2(cos(b)) * p2(y) / 8;
		ret -= 9 * p4(cos(b)) * p4(y) / 64;
		ret -= 65 * p6(cos(b)) * p6(y) / 1024;
		return ret;
	};
	Vector3f axis = get_rotation_axis();

	for (size_t i = 0; i < tiles.size(); i += 1)
	{
		Tile& tile = tiles[i];
		Vector3f dt = normalize(tile.center);
		f32 theta = angle(axis, dt);
		theta = theta - PIf / 2;
		f32 beta = axial_tilt * DEG_RADf;
		f32 y = sinf(theta);

		f32 intensity = sig(y, beta);
		tile.heat_quantity = intensity * 10 + generation_param.average_temperature;
		intensity += generation_param.average_temperature;
		f32 dividor = 1.0f;
		switch (tile.kind)
		{
		case Tile::Kind::DEEP_OCEAN:
			dividor = 1.05;
			break;
		case Tile::Kind::SHALLOW_OCEAN:
			dividor = 1.01;
			break;
		case Tile::Kind::BEACH:
			dividor = 1.005;
			break;
		case Tile::Kind::FOREST:
			dividor = 0.95;
			break;
		case Tile::Kind::PEAK:
			dividor = 1.3;
			break;
		case Tile::Kind::COUNT:
			break;
		}
		intensity /= 1 + (dividor - 1) / 75;
		intensity = 10 * (intensity - generation_param.average_temperature);
		intensity += generation_param.average_temperature;

		tile.year_temperature = intensity;
		min_year_temp = std::min(min_year_temp, intensity);
		max_year_temp = std::max(max_year_temp, intensity);
	}
}

void Planet::fill_base_pressure() {
	f32 min_p = +FLT_MAX;
	f32 max_p = -FLT_MAX;
	Vector3f axis = get_rotation_axis();
	Vector3f zero = get_zero_longitude_axis();

	auto A = [] (f32 x) -> f32 {
		return std::powf(std::powf(std::sinf(2 * std::abs(x)), 1/3.f), 2.f);
	};
	auto B = [] (f32 x) -> f32 {
		return (0.5 + (1 - std::sinf(x) * std::sinf(x))) / 2;
	};
	auto f = [&] (f32 x) -> f32 {
		return (A(x) + B(x)) / 1.75f;
	};

	for (size_t i = 0; i < tiles.size(); i += 1) {
		Tile& tile = tiles[i];
		Vector3f dt = normalize(tile.center);
		f32 theta = angle(axis, dt);
		theta = theta - PIf / 2;
		f32 y = 1.f - cosf(theta * 6.f);
		f32 x = cosf(6.f * angle(zero, normalize(dt - axis * dot(dt, axis))));

		f32 t = tile.heat_quantity;

		f32 p = 0.287 * t / 5;
		f32 factorAlt = std::powf(
			1 - std::clamp(6.87535f * 0.000001f * 3281 * std::max(tile.height, 0.f), 0.f, 1.f),
			5.2561f
		) / 30;
		f32 factorTilt = f(theta);
		f32 factorLL = y * ((cos(theta) * cos(theta)) * 0.25f * x + 0.5f);
		tile.base_pressure = p * factorAlt + factorLL;
	}
}

void Planet::fill_macro_wind() {
	for (size_t i = 0; i < tiles.size(); i += 1)
	{
		f32 curr = tiles[i].base_pressure;
		f32 a = tiles[tiles[i].na].base_pressure;
		f32 b = tiles[tiles[i].nb].base_pressure;
		f32 c = tiles[tiles[i].nc].base_pressure;

		Vector3f da = normalize(tiles[tiles[i].na].center - tiles[i].center);
		Vector3f db = normalize(tiles[tiles[i].nb].center - tiles[i].center);
		Vector3f dc = normalize(tiles[tiles[i].nc].center - tiles[i].center);

		tiles[i].macro_wind = normalize((curr - a) * da + (curr - b) * db + (curr - c) * dc);
	}
}

void Planet::fill_wind_step_to_moutain() {
	std::vector<size_t> open;
	std::vector<float> weights;

	f32 max_height = 0.f;
	for (size_t i = 0; i < tiles.size(); i += 1) {
		max_height = std::max(max_height, tiles[i].height);
	}

	f32 peak_height = max_height * 0.25f;

	for (size_t i = 0; i < tiles.size(); i += 1) {
		Tile& tile = tiles[i];

		open.clear();
		weights.clear();

		open.push_back(i);
		weights.resize(tiles.size(), 0.f);
		weights[i] = 1.f;

		float sum = 0.f;

		size_t cursor = 0;
		while (cursor < open.size()) {
			size_t idx = open[cursor];
			cursor += 1;

			float w = weights[idx];
			weights[idx] = 0;
			if (w < 0.01f)
				continue;

			if (tiles[idx].height > 0 && std::sqrt(tiles[idx].height / max_height) > 0.3f) {
				sum += w;
				continue;
			}

			Vector3f dt = tiles[idx].macro_wind * -1.f;
			Vector3f da = normalize(tiles[tiles[idx].na].center - tiles[idx].center);
			Vector3f db = normalize(tiles[tiles[idx].nb].center - tiles[idx].center);
			Vector3f dc = normalize(tiles[tiles[idx].nc].center - tiles[idx].center);

			float sa = std::max((dot(da, tile.macro_wind) + 0.6f) / 1.6f, 0.f);
			float sb = std::max((dot(db, tile.macro_wind) + 0.6f) / 1.6f, 0.f);
			float sc = std::max((dot(dc, tile.macro_wind) + 0.6f) / 1.6f, 0.f);
			float dsum = sa + sb + sc;

			if (sa > 0) {
				weights[tiles[idx].na] += w * sa / dsum * 0.975f;
				open.push_back(tiles[idx].na);
			}
			if (sb > 0) {
				weights[tiles[idx].nb] += w * sb / dsum * 0.975f;
				open.push_back(tiles[idx].nb);
			}
			if (sc > 0) {
				weights[tiles[idx].nc] += w * sc / dsum * 0.975f;
				open.push_back(tiles[idx].nc);
			}
		}

		tile.wind_step_to_moutain = sum;
	}

	f32 ma = -FLT_MAX;
	f32 mi = +FLT_MAX;

	for (size_t i = 0; i < tiles.size(); i += 1) {
		Tile& tile = tiles[i];
		ma = std::max(ma, tile.wind_step_to_moutain);
		mi = std::min(mi, tile.wind_step_to_moutain);
	}

	for (size_t i = 0; i < tiles.size(); i += 1) {
		Tile& tile = tiles[i];
		tile.wind_step_to_moutain = (tile.wind_step_to_moutain - mi) / (ma - mi);
	}
}

void Planet::fill_humidity() {
	std::vector<size_t> open;
	std::vector<float> weights;

	for (size_t i = 0; i < tiles.size(); i += 1) {
		Tile& tile = tiles[i];

		open.clear();
		weights.clear();

		open.push_back(i);
		weights.resize(tiles.size(), 0.f);
		weights[i] = 1.f;

		float sum = 0.f;

		size_t cursor = 0;
		while (cursor < open.size()) {
			size_t idx = open[cursor];
			cursor += 1;

			float w = weights[idx];
			weights[idx] = 0;
			if (w < 0.01f)
				continue;

			if (
				tiles[idx].kind == Tile::Kind::SHALLOW_OCEAN ||
				tiles[idx].kind == Tile::Kind::DEEP_OCEAN
			) {
				sum += w;
				continue;
			}

			Vector3f dt = tiles[idx].macro_wind * -1.f;
			Vector3f da = normalize(tiles[tiles[idx].na].center - tiles[idx].center);
			Vector3f db = normalize(tiles[tiles[idx].nb].center - tiles[idx].center);
			Vector3f dc = normalize(tiles[tiles[idx].nc].center - tiles[idx].center);

			float sa = std::max((dot(da, tile.macro_wind) + 0.4f) / 1.4f, 0.f);
			float sb = std::max((dot(db, tile.macro_wind) + 0.4f) / 1.4f, 0.f);
			float sc = std::max((dot(dc, tile.macro_wind) + 0.4f) / 1.4f, 0.f);
			float dsum = sa + sb + sc;

			if (sa > 0) {
				weights[tiles[idx].na] += w * sa / dsum * 0.95f;
				open.push_back(tiles[idx].na);
			}
			if (sb > 0) {
				weights[tiles[idx].nb] += w * sb / dsum * 0.95f;
				open.push_back(tiles[idx].nb);
			}
			if (sc > 0) {
				weights[tiles[idx].nc] += w * sc / dsum * 0.95f;
				open.push_back(tiles[idx].nc);
			}
		}

		tile.humidity = sum;
	}

	f32 max_hu = -FLT_MAX;
	f32 min_hu = +FLT_MAX;
	for (size_t i = 0; i < tiles.size(); i += 1) {
		Tile& tile = tiles[i];
		max_hu = std::max(max_hu, tile.humidity);
		min_hu = std::min(min_hu, tile.humidity);
	}
	for (size_t i = 0; i < tiles.size(); i += 1) {
		Tile& tile = tiles[i];
		tile.humidity = (tile.humidity - min_hu) / (max_hu - min_hu);
	}

	for (size_t i = 0; i < tiles.size(); i += 1) {
		Tile& tile = tiles[i];
		tile.humidity = (tile.humidity * 0.8 + 0.2) * tile.year_temperature;
		tile.humidity -= tile.height / 20;
	}

	if (true) for (size_t j = 0; j < 4; j += 1) {
		std::vector<float> temp_humidity;
		temp_humidity.resize(tiles.size());

		for (size_t i = 0; i < tiles.size(); i += 1) {
			Tile& tile = tiles[i];

			float ha = tiles[tile.na].humidity;
			float hb = tiles[tile.nb].humidity;
			float hc = tiles[tile.nc].humidity;

			temp_humidity[i] = (tile.humidity + (ha + hb + hc) / 3.f) / 2.f;
		}

		for (size_t i = 0; i < tiles.size(); i += 1) {
			Tile& tile = tiles[i];
			tile.humidity = temp_humidity[i];
		}
	}
	for (size_t i = 0; i < tiles.size(); i += 1) {
		Tile& tile = tiles[i];
		tile.humidity *= ((1.f - std::sqrt(std::sqrt(tile.wind_step_to_moutain))) * 0.5 + 0.25);
	}

	for (size_t i = 0; i < tiles.size(); i += 1) {
		Tile& tile = tiles[i];
		if (
			tile.kind != Tile::Kind::DEEP_OCEAN &&
			tile.kind != Tile::Kind::SHALLOW_OCEAN
		) {
			max_hu = std::max(max_hu, tile.humidity);
			min_hu = std::min(min_hu, tile.humidity);
		}
	}

	for (size_t i = 0; i < tiles.size(); i += 1) {
		Tile& tile = tiles[i];
		if (tile.kind == Tile::Kind::DEEP_OCEAN)
			tile.humidity = 1.f;
		else if (tile.kind == Tile::Kind::SHALLOW_OCEAN)
			tile.humidity = 1.f;
		else
			tile.humidity = (tile.humidity - min_hu) / (max_hu - min_hu);
	}
}


void Planet::Mesh::upload(SDL_GPUDevice* gpu, std::vector<SDL_GPUFence*>& fences) {
	void* gpu_data = SDL_MapGPUTransferBuffer(gpu, gpu_transfer_buffer, false);
	memcpy(gpu_data, vertices.data(), vertices.size() * sizeof(*vertices.data()));
	SDL_UnmapGPUTransferBuffer(gpu, gpu_transfer_buffer);

	SDL_GPUCommandBuffer* buffer = SDL_AcquireGPUCommandBuffer(gpu);
	SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(buffer);

	SDL_UploadToGPUBuffer(
		copy,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = gpu_transfer_buffer,
			.offset = 0
		},
		&(SDL_GPUBufferRegion) {
			.buffer = gpu_vertex_buffer,
			.offset = 0,
			.size = (u32)(vertices.size() * sizeof(*vertices.data()))
		},
		false
	);

	SDL_EndGPUCopyPass(copy);
	fences.push_back(SDL_SubmitGPUCommandBufferAndAcquireFence(buffer));
}

bool Planet::Mesh::create_pipeline(SDL_GPUDevice* gpu, SDL_GPUTextureFormat format) {
	SDL_GPUShader* vertex_shader = nullptr;
	SDL_GPUShader* fragment_shader = nullptr;
	{
		size_t vertex_size = 0;
		void* plain_vertex = SDL_LoadFile("assets/shaders/vert_planet.spv", &vertex_size);
		defer {
			SDL_free(plain_vertex);
		};

		size_t fragment_size = 0;
		void* plain_fragment = SDL_LoadFile("assets/shaders/frag_planet.spv", &fragment_size);
		defer {
			SDL_free(plain_fragment);
		};

		if (!plain_vertex || !plain_fragment) {
			printf("Failed to load shader files: %s\n", SDL_GetError());
			return false;
		}

		SDL_GPUShaderCreateInfo vertex_info = {};
		vertex_info.code_size = vertex_size;
		vertex_info.code = (u8*)plain_vertex;
		vertex_info.entrypoint = "main";
		vertex_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
		vertex_info.stage = SDL_GPU_SHADERSTAGE_VERTEX;
		vertex_info.num_samplers = 0;
		vertex_info.num_storage_textures = 0;
		vertex_info.num_storage_buffers = 0;
		vertex_info.num_uniform_buffers = 2;
		vertex_shader = SDL_CreateGPUShader(gpu, &vertex_info);

		SDL_GPUShaderCreateInfo fragment_info = {};
		fragment_info.code_size = fragment_size;
		fragment_info.code = (u8*)plain_fragment;
		fragment_info.entrypoint = "main";
		fragment_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
		fragment_info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
		fragment_info.num_samplers = 0;
		fragment_info.num_storage_textures = 0;
		fragment_info.num_storage_buffers = 0;
		fragment_info.num_uniform_buffers = 2;
		fragment_shader = SDL_CreateGPUShader(gpu, &fragment_info);
	}

	if (!vertex_shader || !fragment_shader) {
		printf("Failed to create shaders: %s\n", SDL_GetError());
		return false;
	}

	SDL_GPUColorTargetDescription color_target_desc[] = {
		{
			.format = format,
		}
	};
	SDL_GPUVertexBufferDescription vertex_buffer_desc[] = {
		{
			.slot = 0,
			.pitch = sizeof(Vertex),
			.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX
		}
	};
	SDL_GPUVertexAttribute vertex_attributes[] = {
		{
			.location = 0,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = offsetof(Vertex, position),
		},
		{
			.location = 1,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = offsetof(Vertex, normal),
		},
		{
			.location = 2,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT,
			.offset = offsetof(Vertex, scalar),
		},
		{
			.location = 3,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_UINT,
			.offset = offsetof(Vertex, palette_index),
		},
		{
			.location = 4,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_UINT,
			.offset = offsetof(Vertex, triangle_index),
		}
	};
	SDL_GPUGraphicsPipelineCreateInfo info = {
		.vertex_shader = vertex_shader,
		.fragment_shader = fragment_shader,
		.vertex_input_state = {
			.vertex_buffer_descriptions = vertex_buffer_desc,
			.num_vertex_buffers = sizeof(vertex_buffer_desc) / sizeof(*vertex_buffer_desc),
			.vertex_attributes = vertex_attributes,
			.num_vertex_attributes = sizeof(vertex_attributes) / sizeof(*vertex_attributes),
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.rasterizer_state = {
			.fill_mode = SDL_GPU_FILLMODE_FILL,
			.cull_mode = SDL_GPU_CULLMODE_NONE,
			.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
		},
		.multisample_state = {
			.sample_count = SDL_GPU_SAMPLECOUNT_8,
		},
		.depth_stencil_state = {
			.compare_op = SDL_GPU_COMPAREOP_LESS,
			.write_mask = 0xFF,
			.enable_depth_test = true,
			.enable_depth_write = true,
			.enable_stencil_test = false,
		},
		.target_info = {
			.color_target_descriptions = color_target_desc,
			.num_color_targets = sizeof(color_target_desc) / sizeof(*color_target_desc),
			.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
			.has_depth_stencil_target = true,
		},
	};
	if (pipeline) {
		SDL_ReleaseGPUGraphicsPipeline(gpu, pipeline);
	}
	pipeline = SDL_CreateGPUGraphicsPipeline(gpu, &info);
	if (!pipeline) {
		printf("Failed to create graphics pipeline: %s\n", SDL_GetError());
		return false;
	}

	SDL_ReleaseGPUShader(gpu, vertex_shader);
	SDL_ReleaseGPUShader(gpu, fragment_shader);
	return true;
}

void Planet::imgui(SDL_GPUDevice* gpu) {
	bool need_regen = false;
	
	int x = order;
	need_regen |= ImGui::InputInt("Order", &x);
	if (x < 1)
		x = 1;
	order = x;

	x = generation_param.octave;
	need_regen |= ImGui::InputInt("Octaves", &x);
	if (x < 1)
		x = 1;
	generation_param.octave = x;

	need_regen |= ImGui::SliderFloat("Roughness", &generation_param.roughness, 0.0f, 1.0f);
	need_regen |= ImGui::SliderFloat("Lacunarity", &generation_param.lacunarity, 0.0f, 10.0f);
	need_regen |= ImGui::SliderFloat("Water level", &generation_param.water_level, 0.0f, 1.0f);
	need_regen |= ImGui::SliderFloat("Peak level", &generation_param.peak_level, 0.0f, 1.0f);

	x = generation_param.n_plates;
	need_regen |= ImGui::InputInt("Plates", &x);
	if (x < 1)
		x = 1;
	generation_param.n_plates = x;

	x = generation_param.plate_fail_smooth;
	need_regen |= ImGui::InputInt("Plate fail smooth", &x);
	if (x < 1)
		x = 1;
	generation_param.plate_fail_smooth = x;

	need_regen |= ImGui::SliderFloat(
		"Plate fail smooth factor", &generation_param.plate_fail_smooth_factor, 0.0f, 1.0f
	);

	need_regen |= ImGui::SliderFloat("Plate speed", &generation_param.plate_speed, 0.0f, 10.0f);

	ImGui::SeparatorText("Palette");

	for (size_t i = 0; i < 8; i += 1) {
		ImGui::PushID(i);
		defer {
			ImGui::PopID();
		};
		ImGui::ColorEdit3("Color", (f32*)&uniform.palette[i]);
	}

	bool need_recategorize = false;

	need_recategorize |= ImGui::SliderFloat("Min temp desert °C", &min_temp_desert, 0, 50);
	need_recategorize |= ImGui::SliderFloat("Max temp tunra °C", &max_temp_tundra, 0, 50);
	need_recategorize |= ImGui::SliderFloat("Max snow temp °C", &max_snow_temp, 0, 50);
	need_recategorize |= ImGui::SliderFloat("Max ice temp °C", &max_ice_temp, 0, 50);
	need_recategorize |= ImGui::SliderFloat("Desert Humi", &humidity_desert, 0, 1);
	need_recategorize |= ImGui::SliderFloat("Steppe Humi", &humidity_steppe, 0, 1);
	need_recategorize |= ImGui::SliderFloat("Rainforest Humi", &humidity_rainforest, 0, 1);
	need_recategorize |= ImGui::SliderFloat("Snow peak factor", &snow_peak_factor, 0, 10);

	ImGui::SeparatorText("Orbit");

	ImGui::SliderFloat("Radius", &radius, 0.1f, 10.0f);
	ImGui::SliderFloat("Day period", &day_period, 0.1f, 100.0f);
	ImGui::SliderFloat("Year period", &year_period, 0.1f, 1000.0f);
	ImGui::SliderFloat("Orbit ecentricity", &orbit_ecentricity, 0.0f, 1.0f);
	ImGui::SliderFloat("Orbit inclination", &orbit_inclination, 0.0f, 90.0f);
	need_regen |= ImGui::SliderFloat("Axial tilt", &axial_tilt, 0.0f, 90.0f);


	ImGui::SeparatorText("Overlay");
	{
		int x = (int)overlay_render;
		ImGui::Combo(
			"Render",
			&x,
			"None\0"
			"Height\0"
			"Water distance\0"
			"Tectionic plates\0"
			"Temperature\0"
			"Pressure\0"
			"Macro wind\0"
			"Wind step to moutain\0"
			"Humidity\0"
			"HeatQuantity\0"
		);
		uniform.overlay = x;
		overlay_render = (Overlay_Render)x;
	}

	ImGui::SeparatorText("Info");

	ImGui::Text("Time: % 5.2f", time);
	ImGui::Text("Position % 5.2f % 5.2f % 5.2f", position.x, position.y, position.z);

	if (need_regen) {
		generate_icosphere(gpu, order);
		generate_from_mesh(generation_param);
	}
	if (need_recategorize) {
		find_water(generation_param.water_level, generation_param.peak_level);
		categorize_tiles();
		final_categorize_tiles();
	}
}

void Planet::update(f32 dt)
{
	time += dt;
	time_day += dt;
	time_year += dt;

	time_day = std::fmod(time_day, day_period);
	time_year = std::fmod(time_year, year_period);

	f32 t_day = time_day / day_period;
	f32 t_year = time_year / year_period;

	f32 angle = 2 * PIf * t_day;
	Vector3f axis = get_rotation_axis();
	Quaternionf q_day = Quaternionf::axis_angle(axis, angle);

	angle = 2 * PIf * t_year;
	Quaternionf q_year = Quaternionf::axis_angle({ 0, 0, 1 }, angle);

	orientation = q_day;

	position = { radius, 0, 0 };
	position = q_year * position;

	mesh.local = translation(position) * to_rotation_matrix(orientation);

	for (size_t i = 0; i < mesh.vertices.size(); i += 1) {
		mesh.vertices[i].palette_index = (u32)tiles[i / 3].kind;
	}

	render_vector_field = false;
	switch (overlay_render) {
		case Overlay_Render::Height: {
			f32 max_height = -1000;
			f32 min_height = +1000;
			for (size_t i = 0; i < tiles.size(); i += 1) {
				max_height = std::max(max_height, tiles[i].height);
				min_height = std::min(min_height, tiles[i].height);
			}
			
			for (size_t i = 0; i < mesh.vertices.size(); i += 1) {
				f32 h = tiles[i / 3].height;
				mesh.vertices[i].scalar = h / (max_height - min_height);
			}
			break;
		}

		case Overlay_Render::WaterDistance: {
			size_t max_distance = 0;
			for (size_t i = 0; i < tiles.size(); i += 1)
				max_distance = std::max(max_distance, tiles[i].distanceToWater);
			for (size_t i = 0; i < mesh.vertices.size(); i += 1) {
				mesh.vertices[i].scalar = tiles[i / 3].distanceToWater / (f32)max_distance;
			}
			break;
		}
		case Overlay_Render::Temperature: {
			f32 max_t = -FLT_MAX;
			f32 min_t = +FLT_MAX;
			for (size_t i = 0; i < tiles.size(); i += 1) {
				max_t = std::max(max_t, tiles[i].year_temperature);
				min_t = std::min(min_t, tiles[i].year_temperature);
			}
			for (size_t i = 0; i < mesh.vertices.size(); i += 1) {
				f32 t = tiles[i / 3].year_temperature;
				mesh.vertices[i].scalar = (t - min_t) / (max_t - min_t);
			}
			break;
		}
		case Overlay_Render::TectonicPlates: {
			for (size_t i = 0; i < mesh.vertices.size(); i += 1) {
				mesh.vertices[i].scalar = tiles[i / 3].plate_index / (f32)generation_param.n_plates;
			}

			break;
		}
		case Overlay_Render::MacroWind:
			render_vector_field = true;
		case Overlay_Render::Pressure: {
			f32 mi = +FLT_MAX;
			f32 ma = -FLT_MAX;
			for (size_t i = 0; i < tiles.size(); i += 1) {
				mi = std::min(tiles[i].base_pressure, mi);
				ma = std::max(tiles[i].base_pressure, ma);
			}

			for (size_t i = 0; i < mesh.vertices.size(); i += 1) {
				mesh.vertices[i].scalar = (tiles[i / 3].base_pressure - mi) / (ma - mi);
			}
			break;
		}
		case Overlay_Render::WindStepToMoutain: {
			f32 mi = +FLT_MAX;
			f32 ma = -FLT_MAX;
			for (size_t i = 0; i < tiles.size(); i += 1) {
				mi = std::min(tiles[i].wind_step_to_moutain, mi);
				ma = std::max(tiles[i].wind_step_to_moutain, ma);
			}

			for (size_t i = 0; i < mesh.vertices.size(); i += 1) {
				mesh.vertices[i].scalar = (tiles[i / 3].wind_step_to_moutain - mi) / (ma - mi);
			}
			break;
		}
		case Overlay_Render::Humidity: {
			f32 mi = +FLT_MAX;
			f32 ma = -FLT_MAX;
			for (size_t i = 0; i < tiles.size(); i += 1) {
				mi = std::min(tiles[i].humidity, mi);
				ma = std::max(tiles[i].humidity, ma);
			}

			for (size_t i = 0; i < mesh.vertices.size(); i += 1) {
				mesh.vertices[i].scalar = (tiles[i / 3].humidity - mi) / (ma - mi);
			}
			break;
		}
		case Overlay_Render::HeatQuantity: {
			f32 mi = +FLT_MAX;
			f32 ma = -FLT_MAX;
			for (size_t i = 0; i < tiles.size(); i += 1) {
				mi = std::min(tiles[i].heat_quantity, mi);
				ma = std::max(tiles[i].heat_quantity, ma);
			}

			for (size_t i = 0; i < mesh.vertices.size(); i += 1) {
				mesh.vertices[i].scalar = (tiles[i / 3].heat_quantity - mi) / (ma - mi);
			}
			break;
		}

		default:
			for (size_t i = 0; i < mesh.vertices.size(); i += 1)
				mesh.vertices[i].scalar = 0.0f;
			break;
	}
}


void Planet::upload(SDL_GPUDevice* gpu, std::vector<SDL_GPUFence*>& fences) {
	if (render_vector_field) {
		if (!vector_field.vertex_buffer)
			vector_field.upload(gpu, fences);
		switch (overlay_render) {
			case Overlay_Render::MacroWind: {
				std::vector<WorldArrow::Instance> instances;
				instances.resize(tiles.size());
				for (size_t i = 0; i < tiles.size(); i += 1)
				{
					WorldArrow::Instance& instance = instances[i];
					instance.color = Vector3f(1.f, 1.f, 1.f);
					instance.dir = tiles[i].macro_wind;
					instance.pos = tiles[i].center * 1.001f;
					instance.scale = 0.003f;
					instance.up = normalize(tiles[i].center);
				}
				vector_field.set_instances(gpu, instances.data(), instances.size(), fences);
				break;
			}
			case Overlay_Render::None:
			case Overlay_Render::Height:
			case Overlay_Render::WaterDistance:
			case Overlay_Render::TectonicPlates:
			case Overlay_Render::Temperature:
			case Overlay_Render::Pressure:
			case Overlay_Render::WindStepToMoutain:
			case Overlay_Render::Humidity:
			case Overlay_Render::HeatQuantity:
			case Overlay_Render::Count:
				break;
		}
	}
	mesh.upload(gpu, fences);
}

void Planet::render(SDL_GPURenderPass* pass, SDL_GPUCommandBuffer* command) {

	SDL_BindGPUGraphicsPipeline(pass, mesh.pipeline);
	SDL_BindGPUVertexBuffers(pass, 0, &(SDL_GPUBufferBinding) {
		.buffer = mesh.gpu_vertex_buffer,
		.offset = 0
	}, 1);

	common_uniform.model = mesh.local;

	SDL_PushGPUVertexUniformData(command, 0, &common_uniform, sizeof(common_uniform));
	SDL_PushGPUFragmentUniformData(command, 0, &common_uniform, sizeof(common_uniform));

	SDL_PushGPUVertexUniformData(command, 1, &uniform, sizeof(uniform));
	SDL_PushGPUFragmentUniformData(command, 1, &uniform, sizeof(uniform));

	SDL_DrawGPUPrimitives(pass, mesh.vertices.size(), 1, 0, 0);

	if (render_vector_field)
	{
		vector_field.common_uniform = common_uniform;
		vector_field.render(pass, command);
	}
}


void Planet::find_water(f32 water_level, f32 peak_level) {
	std::vector<size_t> indices;
	indices.resize(mesh.vertices.size() / 3);
	for (size_t i = 0; i < indices.size(); i += 1) {
		indices[i] = i;
	}

	Vector3f axis = { 0, 0, 1 };
	Quaternionf q_start_tilt = Quaternionf::axis_angle({ 1, 0, 0 }, axial_tilt * DEG_RADf);
	axis = q_start_tilt * axis;

	std::sort(std::begin(indices), std::end(indices), [&] (size_t a, size_t b) {
		Vector3f ca = mesh.vertices[a * 3].position;
		ca = ca + mesh.vertices[a * 3 + 1].position;
		ca = ca + mesh.vertices[a * 3 + 2].position;
		ca = normalize(ca);
		Vector3f cb = mesh.vertices[b * 3].position;
		cb = cb + mesh.vertices[b * 3 + 1].position;
		cb = cb + mesh.vertices[b * 3 + 2].position;
		cb = normalize(cb);
		
		f32 ha = tiles[a].height;
		f32 hb = tiles[b].height;

		f32 da = dot(ca, axis);
		f32 db = dot(cb, axis);

		ha *= (1.f - da * da) * 100.f;
		hb *= (1.f - db * db) * 100.f;

		return ha < hb;
	});

	std::vector<u8> is_water(tiles.size(), 0);
	{
		for (size_t i = 0; i < indices.size(); i += 1) {
			tiles[indices[i]].kind = Tile::Kind::COUNT;
		}
		size_t i = 0;
		for (; i < 0.9 * water_level * indices.size() && i < indices.size(); i += 1) {
			is_water[indices[i]] = 1;
			tiles[indices[i]].kind = Tile::Kind::DEEP_OCEAN;
		}
		for (; i < 1.0 * water_level * indices.size() && i < indices.size(); i += 1) {
			is_water[indices[i]] = 1;
			tiles[indices[i]].kind = Tile::Kind::SHALLOW_OCEAN;
		}
		i = std::max(i, (size_t)(peak_level * indices.size()));
		for (; i < indices.size(); i += 1) {
			tiles[indices[i]].kind = Tile::Kind::PEAK;
		}
	}

	// We will do a multi-source breadth-first fill to count the distance to the nearest water tile
	std::vector<size_t> open;
	std::vector<std::uint8_t> closed;
	closed.resize(tiles.size(), 0);

	// Seed with water tiles
	for (size_t i = 0; i < tiles.size(); i += 1) {
		if (is_water[i]) {
			open.push_back(i);
			closed[i] = 1;
			tiles[i].distanceToWater = 0;
			tiles[i].nextTileToWater = SIZE_MAX;
		} else {
			tiles[i].distanceToWater = SIZE_MAX;
			tiles[i].nextTileToWater = SIZE_MAX;
		}
	}

	size_t cursor = 0;

	while (cursor < open.size()) {
		size_t i = open[cursor];
		cursor += 1;

		Tile& tile = tiles[i];

		size_t na = tile.na;
		size_t nb = tile.nb;
		size_t nc = tile.nc;

		if (na != SIZE_MAX && !closed[na]) {
			open.push_back(na);
			closed[na] = 1;
			tiles[na].distanceToWater = tile.distanceToWater + 1;
			tiles[na].nextTileToWater = i;
		}
		if (nb != SIZE_MAX && !closed[nb]) {
			open.push_back(nb);
			closed[nb] = 1;
			tiles[nb].distanceToWater = tile.distanceToWater + 1;
			tiles[nb].nextTileToWater = i;
		}
		if (nc != SIZE_MAX && !closed[nc]) {
			open.push_back(nc);
			closed[nc] = 1;
			tiles[nc].distanceToWater = tile.distanceToWater + 1;
			tiles[nc].nextTileToWater = i;
		}
	}
}

void Planet::categorize_tiles() {
	size_t max_distance = 0;
	for (size_t i = 0; i < tiles.size(); i += 1) {
		max_distance = std::max(max_distance, tiles[i].distanceToWater);
	}

	size_t peak_distance = (max_distance * 9) / 10;

	for (size_t i = 0; i < tiles.size(); i += 1) {
		if (tiles[i].kind != Tile::Kind::COUNT) {
			continue;
		}

		if (tiles[i].distanceToWater > 0 && tiles[i].distanceToWater < 3) {
			tiles[i].kind = Tile::Kind::BEACH;
		} else if (tiles[i].distanceToWater > 0) {
			tiles[i].kind = Tile::Kind::FOREST;
		}
	}
}

void Planet::final_categorize_tiles() {
	for (size_t i = 0; i < tiles.size(); i += 1) {
		Tile& tile = tiles[i];

		if (tile.kind == Tile::Kind::DEEP_OCEAN){
			if (tile.year_temperature < max_ice_temp) {
				tile.kind = Tile::Kind::SNOW;
			}
			continue;
		}
		if (tile.kind == Tile::Kind::SHALLOW_OCEAN)
		{
			if (tile.year_temperature < max_snow_temp) {
				tile.kind = Tile::Kind::ICE;
			}
			continue;
		}
		if (tile.year_temperature < max_snow_temp) {
			tile.kind = Tile::Kind::SNOW;
			continue;
		}
		if (tile.kind == Tile::Kind::BEACH)
			continue;
		if (tile.kind == Tile::Kind::PEAK)
		{
			if (tile.height * snow_peak_factor > tile.year_temperature)
				tile.kind = Tile::Kind::SNOW_PEAK;
			continue;
		}

		if (tile.humidity < humidity_desert) {
			if (tile.year_temperature > min_temp_desert) {
				tile.kind = Tile::Kind::DESERT;
			} else if (tile.year_temperature < max_temp_tundra) {
				tile.kind = Tile::Kind::TUNDRA;
			}
		}
		else if (tile.humidity < humidity_steppe) {
			tile.kind = Tile::Kind::STEPPE;
		}
		else if (tile.humidity > humidity_rainforest) {
			tile.kind = Tile::Kind::RAIN_FOREST;
		}
	}
}


void Planet::grow_plates(
	size_t n_plates, f32 plate_speed, size_t fail_smooth, f32 fail_smooth_factor
) {
	plates.resize(n_plates);

	std::vector<std::vector<size_t>> grow_plates_open_lists;
	std::vector<size_t> grow_plates_cursors;
	size_t grow_plates_n_visited = 0;

	for (size_t i = 0; i < tiles.size(); i += 1) {
		tiles[i].plate_index = SIZE_MAX;
	}
	grow_plates_open_lists.resize(n_plates);
	grow_plates_cursors.resize(n_plates, 0);

	for (size_t i = 0; i < n_plates; i += 1) {
		f32 y = 1.f - (i / (n_plates - 1.f)) * 2.f;
		f32 t = PIf * (std::sqrtf(5) - 1) * i;
		f32 r = std::sqrtf(1 - y * y);

		f32 x = std::cosf(t) * r;
		f32 z = std::sinf(t) * r;

		Vector3f p = { x, y, z };
		p = normalize(p);

		size_t best_tile = SIZE_MAX;
		f32 best_dot = -1;

		for (size_t j = 0; j < tiles.size(); j += 1) {
			Vector3f q = tiles[j].center;
			f32 d = dot(p, q);
			if (d > best_dot) {
				best_dot = d;
				best_tile = j;
			}
		}

		tiles[best_tile].plate_index = i;
		grow_plates_open_lists[i].push_back(best_tile);
	}

	std::vector<size_t> weights(n_plates, 1);
	for (size_t i = 0; i < n_plates; i += 1) {
		if (::uniform(seed) < 0.25f)
			weights[i] = 2;
		if (::uniform(seed) < 0.05f)
			weights[i] = 3;
	}

	size_t iteration = 0;

	while (grow_plates_n_visited < tiles.size()) {

		for (size_t plate_idx = 0; plate_idx < grow_plates_open_lists.size(); plate_idx += 1) {

			if (grow_plates_cursors[plate_idx] >= grow_plates_open_lists[plate_idx].size()) {
				continue;
			}

			if ((iteration % weights[plate_idx]) != 0) {
				continue;
			}

			size_t i = grow_plates_open_lists[plate_idx][grow_plates_cursors[plate_idx]];
			grow_plates_cursors[plate_idx] += 1;

			Tile& tile = tiles[i];

			size_t na = tile.na;
			size_t nb = tile.nb;
			size_t nc = tile.nc;

			if (na != SIZE_MAX && tiles[na].plate_index == SIZE_MAX) {
				tiles[na].plate_index = plate_idx;
				grow_plates_open_lists[plate_idx].push_back(na);
			}
			if (nb != SIZE_MAX && tiles[nb].plate_index == SIZE_MAX) {
				tiles[nb].plate_index = plate_idx;
				grow_plates_open_lists[plate_idx].push_back(nb);
			}
			if (nc != SIZE_MAX && tiles[nc].plate_index == SIZE_MAX) {
				tiles[nc].plate_index = plate_idx;
				grow_plates_open_lists[plate_idx].push_back(nc);
			}

			grow_plates_n_visited += 1;
		}

		iteration += 1;
	}

	for (size_t i = 0; i < n_plates; i += 1) {
		f32 r = ::uniform(seed);
		f32 t = ::uniform(seed) * 2 * PIf;

		plates[i].angle = t;
		plates[i].speed = r * plate_speed;
	}

	std::vector<f32> fail_lines_dt(tiles.size(), 0);

	// Tweak height on the boundary based on the neighbouring plate divergence.
	for (size_t i = 0; i < tiles.size(); i += 1) {
		size_t pi = tiles[i].plate_index;
		size_t pa = pi;
		size_t pb = pi;
		size_t pc = pi;

		if (tiles[i].na != SIZE_MAX) {
			pa = tiles[tiles[i].na].plate_index;
		}
		if (tiles[i].nb != SIZE_MAX) {
			pb = tiles[tiles[i].nb].plate_index;
		}
		if (tiles[i].nc != SIZE_MAX) {
			pc = tiles[tiles[i].nc].plate_index;
		}

		Vector3f ca = tiles[i].center;
		Vector3f cb = tiles[i].center;
		Vector3f cc = tiles[i].center;
		Vector3f ci = tiles[i].center;
		if (tiles[i].na != SIZE_MAX) {
			ca = tiles[tiles[i].na].center;
		}
		if (tiles[i].nb != SIZE_MAX) {
			cb = tiles[tiles[i].nb].center;
		}
		if (tiles[i].nc != SIZE_MAX) {
			cc = tiles[tiles[i].nc].center;
		}

		f32 si = plates[pi].speed;
		Vector3f vi = { std::cosf(plates[pi].angle), std::sinf(plates[pi].angle), 0 };
		Quaternionf q = Quaternionf::from_unit_vectors({0, 0, 1}, normalize(ci));
		vi = q * vi;
		
		Vector3f va = vi;
		f32 sa = plates[pa].speed;
		if (tiles[i].na != SIZE_MAX) {
			va = { std::cosf(plates[pa].angle), std::sinf(plates[pa].angle), 0 };
			q = Quaternionf::from_unit_vectors({0, 0, 1}, normalize(ca));
			va = q * va;
		}

		Vector3f vb = vi;
		f32 sb = plates[pb].speed;
		if (tiles[i].nb != SIZE_MAX) {
			vb = { std::cosf(plates[pb].angle), std::sinf(plates[pb].angle), 0 };
			q = Quaternionf::from_unit_vectors({0, 0, 1}, normalize(cb));
			vb = q * vb;
		}

		Vector3f vc = vi;
		f32 sc = plates[pc].speed;
		if (tiles[i].nc != SIZE_MAX) {
			vc = { std::cosf(plates[pc].angle), std::sinf(plates[pc].angle), 0 };
			q = Quaternionf::from_unit_vectors({0, 0, 1}, normalize(cc));
			vc = q * vc;
		}

		Vector3f da = normalize(ca - ci);
		Vector3f db = normalize(cb - ci);
		Vector3f dc = normalize(cc - ci);

		f32 div = 0;
		div += (si * dot(vi, da) - sa * dot(va, da));
		div += (si * dot(vi, db) - sb * dot(vb, db));
		div += (si * dot(vi, dc) - sc * dot(vc, dc));
		div /= std::max(3 * si + sa + sb + sc, 0.1f);

		div *= std::abs(div * div);

		div *= (3 * si + sa + sb + sc);
		if (div > 0)
			fail_lines_dt[i] = tiles[i].height * +div;
		else
			fail_lines_dt[i] = tiles[i].height * -div;
	}

	// Smooth out the fail_lines
	for (size_t i = 0; i < fail_smooth; i += 1) {
		std::vector<f32> new_fail_lines = fail_lines_dt;

		for (size_t j = 0; j < tiles.size(); j += 1) {
			size_t a = tiles[j].na;
			size_t b = tiles[j].nb;
			size_t c = tiles[j].nc;

			f32 to_spread = fail_lines_dt[j] * fail_smooth_factor;

			if (a != SIZE_MAX) {
				new_fail_lines[a] += to_spread / 3;
			}
			if (b != SIZE_MAX) {
				new_fail_lines[b] += to_spread / 3;
			}
			if (c != SIZE_MAX) {
				new_fail_lines[c] += to_spread / 3;
			}
		}

		fail_lines_dt = new_fail_lines;
	}

	for (size_t i = 0; i < tiles.size(); i += 1) {
		tiles[i].height += fail_lines_dt[i];
	}
}

Vector3f Planet::get_rotation_axis()
{
	Vector3f axis = { 0, 0, 1 };
	Quaternionf q_start_tilt = Quaternionf::axis_angle({ 1, 0, 0 }, axial_tilt * DEG_RADf);
	axis = q_start_tilt * axis;

	return axis;
}

Vector3f Planet::get_zero_longitude_axis()
{
	Vector3f axis = { 1, 0, 0 };
	Quaternionf q_start_tilt = Quaternionf::axis_angle({ 1, 0, 0 }, axial_tilt * DEG_RADf);
	axis = q_start_tilt * axis;

	return axis;
}
