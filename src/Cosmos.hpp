#pragma once

#include "SDL3/SDL.h"
#include "Graphics.hpp"

struct FullscreenQuad {
	SDL_GPUBuffer* vertex_buffer = nullptr;
	SDL_GPUTransferBuffer* transfer_buffer = nullptr;

	SDL_GPUFence* upload(SDL_GPUDevice* gpu);
	void release(SDL_GPUDevice* gpu);

	static FullscreenQuad quad;
};

struct Cosmos {
	struct Uniform {
		Vector3f empty_color = Vector3f(0.025f, 0.025f, 0.05f);
	};

	size_t n_stars = 1000;

	Uniform uniform;
	Common_Uniform common_uniform;

	SDL_GPUBuffer* shader_buffer = nullptr;
	SDL_GPUGraphicsPipeline* pipeline = nullptr;

	bool create_pipeline(SDL_GPUDevice* gpu);
	void release(SDL_GPUDevice* gpu);

	void imgui();
	void render(SDL_GPURenderPass* pass, SDL_GPUCommandBuffer* command);
};
