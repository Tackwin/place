#include "Cosmos.hpp"
#include "Maths.hpp"
#include <vector>
#include "imgui/imgui.h"


bool Cosmos::create_pipeline(SDL_GPUDevice* gpu) {
	SDL_GPUShader* vertex_shader = nullptr;
	SDL_GPUShader* fragment_shader = nullptr;
	{
		size_t vertex_size = 0;
		void* plain_vertex = SDL_LoadFile("assets/shaders/vert_cosmos.spv", &vertex_size);
		defer {
			SDL_free(plain_vertex);
		};

		size_t fragment_size = 0;
		void* plain_fragment = SDL_LoadFile("assets/shaders/frag_cosmos.spv", &fragment_size);
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
			.format = SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT,
		}
	};
	SDL_GPUVertexBufferDescription vertex_buffer_desc[] = {
		{
			.slot = 0,
			.pitch = sizeof(Vector2f),
			.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX
		}
	};
	SDL_GPUVertexAttribute vertex_attributes[] = {
		{
			.location = 0,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
			.offset = 0,
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
			.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
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
void Cosmos::release(SDL_GPUDevice* gpu){
	if (pipeline) {
		SDL_ReleaseGPUGraphicsPipeline(gpu, pipeline);
	}
	pipeline = nullptr;
}

void Cosmos::imgui() {
	ImGui::ColorEdit3("Empty Color", &uniform.empty_color.x);
}

void Cosmos::render(SDL_GPURenderPass* pass, SDL_GPUCommandBuffer* command) {
	SDL_BindGPUGraphicsPipeline(pass, pipeline);
	SDL_BindGPUVertexBuffers(pass, 0, &(SDL_GPUBufferBinding) {
		.buffer = FullscreenQuad::quad.vertex_buffer,
		.offset = 0
	}, 1);

	SDL_PushGPUVertexUniformData(command, 0, &common_uniform, sizeof(common_uniform));
	SDL_PushGPUFragmentUniformData(command, 0, &common_uniform, sizeof(common_uniform));

	SDL_PushGPUVertexUniformData(command, 1, &uniform, sizeof(uniform));
	SDL_PushGPUFragmentUniformData(command, 1, &uniform, sizeof(uniform));

	SDL_DrawGPUPrimitives(pass, 6, 1, 0, 0);
}
