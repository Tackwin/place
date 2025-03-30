#pragma once

#include "Common.hpp"
#include "Maths.hpp"
#include "SDL3/SDL_gpu.h"
#include <vector>

struct Common_Uniform {
	Matrix4f model;
	Matrix4f view;
	Matrix4f projection;
};

struct FullscreenQuad {
	SDL_GPUBuffer* vertex_buffer = nullptr;
	SDL_GPUTransferBuffer* transfer_buffer = nullptr;

	SDL_GPUFence* upload(SDL_GPUDevice* gpu);
	void release(SDL_GPUDevice* gpu);

	static FullscreenQuad quad;
};

struct WorldArrow {
	SDL_GPUBuffer* vertex_buffer = nullptr;
	SDL_GPUBuffer* instance_buffer = nullptr;
	SDL_GPUTransferBuffer* transfer_buffer = nullptr;
	SDL_GPUTransferBuffer* transfer_instance_buffer = nullptr;
	usz instance_buffer_size = 0;
	usz n_instances = 0;

	struct Uniform {
	};

	Common_Uniform common_uniform;
	Uniform uniform;

	SDL_GPUGraphicsPipeline* pipeline = nullptr;

	void upload(SDL_GPUDevice* gpu, std::vector<SDL_GPUFence*>& fences);
	void release(SDL_GPUDevice* gpu);

	struct Instance {
		Vector3f pos;
		Vector3f dir;
		Vector3f up;
		Vector3f color;
		f32 scale;
	};

	bool create_pipeline(SDL_GPUDevice* gpu, SDL_GPUTextureFormat format);
	void set_instances(
		SDL_GPUDevice* gpu,
		Instance* instance_data,
		usz Instance_size,
		std::vector<SDL_GPUFence*>& fences
	);
	void render(SDL_GPURenderPass* pass, SDL_GPUCommandBuffer* buffer);

	static WorldArrow arrow;
};

struct Postprocess {
	struct Uniform {
		size_t width;
		size_t height;

		size_t hdr;
		size_t samples;

		f32 exposure;
		f32 startCompression;
		f32 desaturation;
	};

	Uniform uniform;

	SDL_GPUBuffer* shader_buffer = nullptr;
	SDL_GPUGraphicsPipeline* pipeline = nullptr;

	bool create_pipeline(SDL_GPUDevice* gpu, SDL_GPUTextureFormat);
	void release(SDL_GPUDevice* gpu);

	void imgui();
	void render(SDL_GPURenderPass* pass, SDL_GPUCommandBuffer* command);
};
