#pragma once

#include "SDL3/SDL.h"
#include "Graphics.hpp"

struct Atmosphere {
	struct Uniform {
		Vector3f eye;
		f32 padding1;
		Vector3f planet_world_position;
		f32 padding2;
		Vector3f color;
		f32 padding3;
		Vector3f ray_beta = Vector3f(70/255.f, 152/255.f, 255/255.f);
		f32 padding4;
		Vector3f mie_beta = Vector3f(255/255.f, 163/255.f, 163/255.f);
		f32 padding5;
		Vector3f ambient_beta = Vector3f(0.5/255.f, 0.5/255.f, 1/255.f);
		f32 padding6;
		Vector3f absorption_beta = Vector3f(255/255.f, 83/255.f, 0/255.f);

		u32 width;
		u32 height;
		f32 planet_radius;
		f32 thickness = 1.0f;
		f32 density = 1.0f;

		f32 height_ray = 0.020f;
		f32 height_mie = 0.010f;
		f32 height_absorption = 10.0f;
		f32 absorption_falloff = 1.0f;
		f32 intensity = 20.f;
	};

	Uniform uniform;
	Common_Uniform common_uniform;


	SDL_GPUBuffer* shader_buffer = nullptr;
	SDL_GPUGraphicsPipeline* pipeline = nullptr;

	bool create_pipeline(SDL_GPUDevice* gpu);
	void release(SDL_GPUDevice* gpu);

	void imgui();
	void render(SDL_GPURenderPass* pass, SDL_GPUCommandBuffer* command);
};