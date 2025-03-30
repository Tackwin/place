#include "Graphics.hpp"
#include "SDL3/SDL_gpu.h"
#include <vector>

FullscreenQuad FullscreenQuad::quad;
SDL_GPUFence* FullscreenQuad::upload(SDL_GPUDevice* gpu) {
	if (!vertex_buffer) {
		vertex_buffer = SDL_CreateGPUBuffer(
			gpu,
			&(SDL_GPUBufferCreateInfo) {
				.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
				.size = 6 * sizeof(Vector2f)
			}
		);
	}
	if (!transfer_buffer) {
		transfer_buffer = SDL_CreateGPUTransferBuffer(
			gpu,
			&(SDL_GPUTransferBufferCreateInfo) {
				.size = 6 * sizeof(Vector2f)
			}
		);
	}

	void* pointer = SDL_MapGPUTransferBuffer(gpu, transfer_buffer, false);

	std::vector<Vector2f> vertices = {
		{ 0, 0 },
		{ 1, 0 },
		{ 0, 1 },
		{ 1, 0 },
		{ 1, 1 },
		{ 0, 1 }
	};

	memcpy(pointer, vertices.data(), vertices.size() * sizeof(*vertices.data()));
	SDL_UnmapGPUTransferBuffer(gpu, transfer_buffer);

	SDL_GPUCommandBuffer* buffer = SDL_AcquireGPUCommandBuffer(gpu);
	SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(buffer);
	SDL_UploadToGPUBuffer(
		copy,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = transfer_buffer,
			.offset = 0
		},
		&(SDL_GPUBufferRegion) {
			.buffer = vertex_buffer,
			.offset = 0,
			.size = (u32)(vertices.size() * sizeof(*vertices.data()))
		},
		false
	);
	SDL_EndGPUCopyPass(copy);

	return SDL_SubmitGPUCommandBufferAndAcquireFence(buffer);
}

void FullscreenQuad::release(SDL_GPUDevice* gpu) {
	if (vertex_buffer) {
		SDL_ReleaseGPUBuffer(gpu, vertex_buffer);
		vertex_buffer = nullptr;
	}
	if (transfer_buffer) {
		SDL_ReleaseGPUTransferBuffer(gpu, transfer_buffer);
		transfer_buffer = nullptr;
	}
}


void WorldArrow::upload(SDL_GPUDevice* gpu, std::vector<SDL_GPUFence*>& fences) {
	if (!vertex_buffer) {
		vertex_buffer = SDL_CreateGPUBuffer(gpu, &(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
			.size = 9 * sizeof(Vector3f)
		});
	}
	if (!transfer_buffer) {
		transfer_buffer = SDL_CreateGPUTransferBuffer(gpu, &(SDL_GPUTransferBufferCreateInfo) {
			.size = 9 * sizeof(Vector3f)
		});
	}
	if (!instance_buffer) {
		instance_buffer_size = 1024;
		instance_buffer = SDL_CreateGPUBuffer(gpu, &(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
			.size = (u32)(instance_buffer_size * sizeof(Instance))
		});
	}
	if (!transfer_instance_buffer) {
		transfer_instance_buffer = SDL_CreateGPUTransferBuffer(gpu, &(SDL_GPUTransferBufferCreateInfo) {
			.size = (u32)(instance_buffer_size * sizeof(Instance))
		});
	}

	void* pointer = SDL_MapGPUTransferBuffer(gpu, transfer_buffer, false);

	std::vector<Vector3f> vertices = {
		{ 0, 0, 0 },
		{ 1, 0, 0 },
		{ 0, 1, 0 },
		{ 1, 0, 0 },
		{ 1, 1, 0 },
		{ 0, 1, 0 },
		{ 1, 0, 0 },
		{ 1, 1, 0 },
		{ 0, 1, 0 }
	};

	memcpy(pointer, vertices.data(), vertices.size() * sizeof(*vertices.data()));
	SDL_UnmapGPUTransferBuffer(gpu, transfer_buffer);

	SDL_GPUCommandBuffer* buffer = SDL_AcquireGPUCommandBuffer(gpu);
	SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(buffer);
	SDL_UploadToGPUBuffer(
		copy,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = transfer_buffer,
			.offset = 0
		},
		&(SDL_GPUBufferRegion) {
			.buffer = vertex_buffer,
			.offset = 0,
			.size = (u32)(vertices.size() * sizeof(*vertices.data()))
		},
		false
	);
	SDL_EndGPUCopyPass(copy);

	fences.push_back(SDL_SubmitGPUCommandBufferAndAcquireFence(buffer));
}

void WorldArrow::release(SDL_GPUDevice* gpu) {
	if (vertex_buffer) {
		SDL_ReleaseGPUBuffer(gpu, vertex_buffer);
		vertex_buffer = nullptr;
	}
	if (transfer_buffer) {
		SDL_ReleaseGPUTransferBuffer(gpu, transfer_buffer);
		transfer_buffer = nullptr;
	}
	if (transfer_instance_buffer) {
		SDL_ReleaseGPUTransferBuffer(gpu, transfer_instance_buffer);
		transfer_instance_buffer = nullptr;
	}
	if (instance_buffer) {
		SDL_ReleaseGPUBuffer(gpu, instance_buffer);
		instance_buffer = nullptr;
	}
}

void WorldArrow::set_instances(
	SDL_GPUDevice* gpu,
	Instance* instance_data,
	usz instance_size,
	std::vector<SDL_GPUFence*>& fences
)
{
	if (instance_buffer_size < instance_size)
	{
		if (instance_buffer)
			SDL_ReleaseGPUBuffer(gpu, instance_buffer);

		instance_buffer_size = std::max(instance_buffer_size * 2, instance_size);
		instance_buffer = SDL_CreateGPUBuffer(gpu, &(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
			.size = (u32)(instance_buffer_size * sizeof(Instance))
		});

		if (transfer_instance_buffer)
			SDL_ReleaseGPUTransferBuffer(gpu, transfer_instance_buffer);

		transfer_instance_buffer = SDL_CreateGPUTransferBuffer(
			gpu, &(SDL_GPUTransferBufferCreateInfo) {
				.size = (u32)(instance_buffer_size * sizeof(Instance))
			}
		);
	}

	void* pointer = SDL_MapGPUTransferBuffer(gpu, transfer_instance_buffer, false);
	memcpy(pointer, instance_data, instance_size * sizeof(Instance));

	SDL_UnmapGPUTransferBuffer(gpu, transfer_instance_buffer);

	SDL_GPUCommandBuffer* buffer = SDL_AcquireGPUCommandBuffer(gpu);
	SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(buffer);
	SDL_UploadToGPUBuffer(
		copy,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = transfer_instance_buffer,
			.offset = 0
		},
		&(SDL_GPUBufferRegion) {
			.buffer = instance_buffer,
			.offset = 0,
			.size = (u32)(instance_size * sizeof(*instance_data))
		},
		false
	);
	SDL_EndGPUCopyPass(copy);
	n_instances = instance_size;
	fences.push_back(SDL_SubmitGPUCommandBufferAndAcquireFence(buffer));
}

bool WorldArrow::create_pipeline(SDL_GPUDevice* gpu, SDL_GPUTextureFormat format)
{
	SDL_GPUShader* vertex_shader = nullptr;
	SDL_GPUShader* fragment_shader = nullptr;
	{
		size_t vertex_size = 0;
		void* plain_vertex = SDL_LoadFile("assets/shaders/vert_arrow.spv", &vertex_size);
		defer {
			SDL_free(plain_vertex);
		};

		size_t fragment_size = 0;
		void* plain_fragment = SDL_LoadFile("assets/shaders/frag_arrow.spv", &fragment_size);
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
			.pitch = sizeof(Vector3f),
			.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX
		},
		SDL_GPUVertexBufferDescription{
			.slot = 1,
			.pitch = sizeof(Instance),
			.input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE,
			.instance_step_rate = 1
		}
	};
	SDL_GPUVertexAttribute vertex_attributes[] = {
		{
			.location = 0,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = 0,
		},
		{
			.location = 1,
			.buffer_slot = 1,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = offsetof(Instance, pos),
		},
		{
			.location = 2,
			.buffer_slot = 1,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = offsetof(Instance, dir),
		},
		{
			.location = 3,
			.buffer_slot = 1,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = offsetof(Instance, up),
		},
		{
			.location = 4,
			.buffer_slot = 1,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = offsetof(Instance, color),
		},
		{
			.location = 5,
			.buffer_slot = 1,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT,
			.offset = offsetof(Instance, scale),
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
	if (pipeline)
		SDL_ReleaseGPUGraphicsPipeline(gpu, pipeline);
	pipeline = SDL_CreateGPUGraphicsPipeline(gpu, &info);
	if (!pipeline) {
		printf("Failed to create graphics pipeline: %s\n", SDL_GetError());
		return false;
	}

	SDL_ReleaseGPUShader(gpu, vertex_shader);
	SDL_ReleaseGPUShader(gpu, fragment_shader);
	return true;
}

void WorldArrow::render(SDL_GPURenderPass* pass, SDL_GPUCommandBuffer* buffer)
{
	SDL_BindGPUGraphicsPipeline(pass, pipeline);
	SDL_BindGPUVertexBuffers(
		pass,
		0,
		(SDL_GPUBufferBinding[]) {
			{
				.buffer = vertex_buffer,
				.offset = 0
			},
			{
				.buffer = instance_buffer,
				.offset = 0
			}
		},
		2
	);

	SDL_PushGPUVertexUniformData(buffer, 0, &common_uniform, sizeof(common_uniform));
	SDL_PushGPUFragmentUniformData(buffer, 0, &common_uniform, sizeof(common_uniform));
	SDL_DrawGPUPrimitives(pass, 9, n_instances, 0, 0);
}

bool Postprocess::create_pipeline(SDL_GPUDevice* gpu, SDL_GPUTextureFormat format)
{
	SDL_GPUShader* vertex_shader = nullptr;
	SDL_GPUShader* fragment_shader = nullptr;
	{
		size_t vertex_size = 0;
		void* plain_vertex = SDL_LoadFile("assets/shaders/vert_post.spv", &vertex_size);
		defer {
			SDL_free(plain_vertex);
		};

		size_t fragment_size = 0;
		void* plain_fragment = SDL_LoadFile("assets/shaders/frag_post.spv", &fragment_size);
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
	},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.rasterizer_state = {
			.fill_mode = SDL_GPU_FILLMODE_FILL,
			.cull_mode = SDL_GPU_CULLMODE_NONE,
			.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
		},
		.multisample_state = {
			.sample_count = SDL_GPU_SAMPLECOUNT_1,
		},
		.depth_stencil_state = {
			.enable_depth_test = false,
			.enable_depth_write = false,
			.enable_stencil_test = false,
		},
		.target_info = {
			.color_target_descriptions = color_target_desc,
			.num_color_targets = sizeof(color_target_desc) / sizeof(*color_target_desc),
			.has_depth_stencil_target = false,
		},
	};
	if (pipeline)
		SDL_ReleaseGPUGraphicsPipeline(gpu, pipeline);
	pipeline = SDL_CreateGPUGraphicsPipeline(gpu, &info);
	if (!pipeline) {
		printf("Failed to create graphics pipeline: %s\n", SDL_GetError());
		return false;
	}

	SDL_ReleaseGPUShader(gpu, vertex_shader);
	SDL_ReleaseGPUShader(gpu, fragment_shader);
	return true;
}

void Postprocess::release(SDL_GPUDevice* gpu)
{
	if (pipeline) {
		SDL_ReleaseGPUGraphicsPipeline(gpu, pipeline);
	}
	pipeline = nullptr;
}

void Postprocess::imgui() {
}
void Postprocess::render(SDL_GPURenderPass* pass, SDL_GPUCommandBuffer* command) {
	SDL_BindGPUGraphicsPipeline(pass, pipeline);
	SDL_BindGPUVertexBuffers(pass, 0, &(SDL_GPUBufferBinding) {
		.buffer = FullscreenQuad::quad.vertex_buffer,
		.offset = 0
	}, 1);

	SDL_PushGPUVertexUniformData(command, 0, &uniform, sizeof(uniform));
	SDL_PushGPUFragmentUniformData(command, 0, &uniform, sizeof(uniform));

	SDL_DrawGPUPrimitives(pass, 6, 1, 0, 0);
}
