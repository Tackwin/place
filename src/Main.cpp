#include "Common.hpp"

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_sdlgpu3.h"

#include <stdio.h>
#include <vector>
#include <optional>
#include <unordered_map>

#include "Maths.hpp"
#include "Noise.hpp"
#include "Planet.hpp"
#include "Cosmos.hpp"
#include "Graphics.hpp"
#include "Atmosphere.hpp"

struct Camera {
	Vector3f position;
	Vector3f target;
	Vector3f up;

	f32 speed = 100.f;

	f32 fov = 45.5f;
	f32 aspect = 16.0f / 9.0f;
	f32 near = 0.1f;
	f32 far = 100.0f;

	Vector3f unproject_ray_ndc(f32 x, f32 y) {
		Vector3f ndc = { x, y, -1.0f };

		Matrix4f proj = perspective(fov, aspect, near, far);
		Matrix4f view = lookAt(position, target, up);

		Vector4f ray_eye = inverse(proj) * Vector4f(ndc, 1.0f);
		ray_eye.z = -1.0f;
		ray_eye.w = 0.0f;

		Vector3f ray_world = (Vector3f)(view * ray_eye);
		return normalize(ray_world);
	}
	Vector3f unproject_ray(f32 x, f32 y) {
		Vector3f ndc = {
			2.0f * x - 1.0f,
			1.0f - 2.0f * y,
			-1.0f
		};
		return unproject_ray_ndc(ndc.x, ndc.y);

	}
};

struct Targets {
	size_t width = 1366;
	size_t height = 768;
	SDL_GPUTexture* color_texture = nullptr;
	SDL_GPUTexture* depth_texture = nullptr;

	SDL_GPUTexture* resolve_texture = nullptr;
};

void destroy_targets(SDL_GPUDevice* gpu, Targets& targets);
bool update_targets(SDL_GPUDevice* gpu, Targets& targets) {
	destroy_targets(gpu, targets);

	SDL_GPUTextureCreateInfo color_texture_info = {
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT,
		.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET,
		.width = (u32)targets.width,
		.height = (u32)targets.height,
		.layer_count_or_depth = 1,
		.num_levels = 1,
		.sample_count = SDL_GPU_SAMPLECOUNT_8,
	};
	targets.color_texture = SDL_CreateGPUTexture(gpu, &color_texture_info);
	if (!targets.color_texture) {
		printf("Failed to create color texture: %s\n", SDL_GetError());
		return false;
	}

	SDL_GPUTextureCreateInfo resolve_texture_info = {
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT,
		.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER,
		.width = (u32)targets.width,
		.height = (u32)targets.height,
		.layer_count_or_depth = 1,
		.num_levels = 1,
		.sample_count = SDL_GPU_SAMPLECOUNT_1,
	};
	targets.resolve_texture = SDL_CreateGPUTexture(gpu, &resolve_texture_info);
	if (!targets.resolve_texture) {
		printf("Failed to create resolve texture: %s\n", SDL_GetError());
		return false;
	}

	SDL_GPUTextureCreateInfo depth_texture_info = {
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
		.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
		.width = (u32)targets.width,
		.height = (u32)targets.height,
		.layer_count_or_depth = 1,
		.num_levels = 1,
		.sample_count = SDL_GPU_SAMPLECOUNT_8,
	};
	targets.depth_texture = SDL_CreateGPUTexture(gpu, &depth_texture_info);
	if (!targets.depth_texture) {
		printf("Failed to create depth texture: %s\n", SDL_GetError());
		return false;
	}
	return true;
}
void destroy_targets(SDL_GPUDevice* gpu, Targets& targets) {
	if (targets.color_texture) {
		SDL_ReleaseGPUTexture(gpu, targets.color_texture);
		targets.color_texture = nullptr;
	}
	if (targets.depth_texture) {
		SDL_ReleaseGPUTexture(gpu, targets.depth_texture);
		targets.depth_texture = nullptr;
	}
	if (targets.resolve_texture) {
		SDL_ReleaseGPUTexture(gpu, targets.resolve_texture);
		targets.resolve_texture = nullptr;
	}
}
#include <filesystem>

void compile_shaders() {
	std::filesystem::path shader_dir = std::filesystem::current_path() / "src" / "shaders";

	// for every file in the shader directory
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(shader_dir))
	{
		// if the file is a file
		if (entry.is_regular_file()) {
			// if the file is a shader file
			if (entry.path().extension() == ".frag") {
				// compile the shader
				std::string output_name = "frag_" + entry.path().filename().replace_extension(".spv").generic_string();
				std::filesystem::path input_path = entry.path();
				std::filesystem::path output_path =
					std::filesystem::current_path() / "assets" / "shaders" / output_name;

				std::string command = "glslc " + input_path.generic_string();
				command += " -o " + output_path.generic_string();
				printf("Compiling fragment shader: %s\n", command.c_str());
				system(command.c_str());
			}
			else if (entry.path().extension() == ".vert") {
				// compile the shader
				std::string output_name = "vert_" + entry.path().filename().replace_extension(".spv").generic_string();
				std::filesystem::path input_path = entry.path();
				std::filesystem::path output_path =
					std::filesystem::current_path() / "assets" / "shaders" / output_name;

				std::string command = "glslc " + input_path.generic_string();
				command += " -o " + output_path.generic_string();
				printf("Compiling vertex shader:   %s\n", command.c_str());
				system(command.c_str());
			}
		}
	}
}


int main(int argc, char** argv) {
	defer {
		SDL_Quit();
	};

	SDL_InitFlags flags = 0;
	flags |= SDL_INIT_VIDEO;
	flags |= SDL_INIT_AUDIO;
	flags |= SDL_INIT_EVENTS;
	
	SDL_SetAppMetadata("Place", "0", "eu.tackwin.place");
	if (!SDL_InitSubSystem(flags))
		return 1;
	defer {
		SDL_QuitSubSystem(flags);
	};

	SDL_Window* window = nullptr;

	SDL_WindowFlags window_flags = 0;
	window_flags |= SDL_WINDOW_RESIZABLE;
	window_flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
	window_flags |= SDL_WINDOW_OPENGL;

	window = SDL_CreateWindow(
		"Place", 1366, 768, window_flags
	);
	if (!window) {
		printf("Failed to create window: %s\n", SDL_GetError());
		return 1;
	}
	defer {
		SDL_DestroyWindow(window);
	};

	SDL_GPUShaderFormat shader_format = SDL_GPU_SHADERFORMAT_SPIRV;
	SDL_GPUDevice* gpu = SDL_CreateGPUDevice(shader_format, true, nullptr);
	if (!gpu) {
		printf("Failed to create GPU device: %s\n", SDL_GetError());
		return 1;
	}
	defer {
		SDL_DestroyGPUDevice(gpu);
	};

	if (!SDL_ClaimWindowForGPUDevice(gpu, window)) {
		printf("Failed to claim window for GPU device: %s\n", SDL_GetError());
		return 1;
	}

	Targets targets;

	if (!update_targets(gpu, targets)) {
		return 1;
	}
	defer {
		destroy_targets(gpu, targets);
	};

	FullscreenQuad::quad.upload(gpu);

	Camera camera = {};
	camera.position = { 0, 0, -2.75 };
	camera.up = { 1, 0, 0 };
	camera.target = { 0, 0, 0 };
	Camera arcball_camera = camera;

	Cosmos cosmos;
	cosmos.create_pipeline(gpu);
	defer {
		cosmos.release(gpu);
	};

	Planet planet;
	planet.generate_icosphere(gpu, planet.order);
	planet.generate_from_mesh(planet.generation_param);
	planet.create_pipeline(gpu, SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT);
	defer {
		planet.release(gpu);
	};

	Atmosphere atmosphere;
	atmosphere.create_pipeline(gpu);
	defer {
		atmosphere.release(gpu);
	};

	Common_Uniform common_uniform = {};

	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplSDL3_InitForSDLGPU(window);
	ImGui_ImplSDLGPU3_InitInfo imgui_info = {};
	imgui_info.Device = gpu;
	imgui_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gpu, window);
	imgui_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
	ImGui_ImplSDLGPU3_Init(&imgui_info);

	bool mouse_down[255] = { };
	bool mouse_up  [255] = { };
	bool mouse_just_down[255] = { };

	std::optional<Vector3f> start_grab = std::nullopt;
	Vector3f end_grab = { 0, 0, 0 };
	Matrix4f start_grab_local = identity();
	Matrix4f grab_local_dt = identity();
	Vector3f start_camera_pos = camera.position;
	Vector3f start_camera_up = camera.up;

	f32 mouse_x = 0;
	f32 mouse_y = 0;
	size_t t0 = SDL_GetPerformanceCounter();

	bool show_planet = true;

	bool is_fullscreen = false;
	bool want_quit = false;
	SDL_Event event;

	bool render_imgui = true;

	ImGuiIO& io = ImGui::GetIO();
	float target_camera_distance = length(camera.position);

	while (!want_quit) {
		size_t t1 = SDL_GetPerformanceCounter();
		f32 dt = (f32)(t1 - t0) / SDL_GetPerformanceFrequency();
		t0 = t1;
		// dt = 1.0f / 144.f;

		memset(mouse_up, 0, sizeof(mouse_up));
		memset(mouse_just_down, 0, sizeof(mouse_just_down));

		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL3_ProcessEvent(&event);

			if (event.type == SDL_EVENT_QUIT) {
				want_quit = true;
				continue;
			}
			if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
				mouse_down[event.button.button] = true;
				mouse_just_down[event.button.button] = true;
				continue;
			}
			if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
				mouse_up[event.button.button] = true;
				mouse_down[event.button.button] = false;
				continue;
			}
			if (event.type == SDL_EVENT_KEY_DOWN) {
				if (event.key.key == SDLK_H) {
					render_imgui = !render_imgui;
				}
			}
			if (event.type == SDL_EVENT_MOUSE_WHEEL && !io.WantCaptureMouse) {
				if (event.wheel.y > 0) {
					Vector3f d = normalize(camera.position - camera.target);
					target_camera_distance /= powf(1.3f, event.wheel.y);
				} else {
					Vector3f d = normalize(camera.position - camera.target);
					target_camera_distance *= powf(1.3f, -event.wheel.y);
				}
			}
			if (event.type == SDL_EVENT_WINDOW_RESIZED) {
				i32 w;
				i32 h;
				bool ok = SDL_GetWindowSizeInPixels(window, &w, &h);
				if (ok && w > 0 && h > 0) {
					targets.width = w;
					targets.height = h;
					if (!update_targets(gpu, targets)) {
						return 1;
					}
				} else {
					printf("Failed to get window size: %s\n", SDL_GetError());
				}
			}
		}

		if (want_quit)
			break;
		if (true)
		{
			ImGui_ImplSDLGPU3_NewFrame();
			ImGui_ImplSDL3_NewFrame();
			ImGui::NewFrame();

			if (show_planet) {
				if (ImGui::Begin("Planet", &show_planet))
					planet.imgui(gpu);
				ImGui::End();
			}

			ImGui::Begin("Cosmos");
			cosmos.imgui();
			ImGui::End();

			ImGui::Begin("Atmosphere");
			atmosphere.imgui();
			ImGui::End();

			ImGui::Begin("Postprocess");
			ImGui::End();

			ImGui::Begin("Debug");
			ImGui::Text("FPS: % 5.2f, MS: % 5.2f ms", 1.0f / dt, (dt * 1000));
			ImGui::Checkbox("Show planet", &show_planet);

			if (ImGui::CollapsingHeader("Camera")) {
				ImGui::SliderFloat("FOV", &camera.fov, 1.0f, 179.0f);
				arcball_camera.fov = camera.fov;

				ImGui::Text(
					"Position % 5.2f % 5.2f % 5.2f",
					camera.position.x,
					camera.position.y,
					camera.position.z
				);
				ImGui::Text(
					"arcball Position % 5.2f % 5.2f % 5.2f",
					arcball_camera.position.x,
					arcball_camera.position.y,
					arcball_camera.position.z
				);

				if (start_grab.has_value()) {
					ImGui::Text(
						"Start grab % 5.2f % 5.2f % 5.2f",
						start_grab->x,
						start_grab->y,
						start_grab->z
					);
					ImGui::Text(
						"End grab % 5.2f % 5.2f % 5.2f",
						end_grab.x,
						end_grab.y,
						end_grab.z
					);
				}
			}
			if (ImGui::CollapsingHeader("Shaders")) {
				if (ImGui::Button("Reload")) {
					compile_shaders();
					planet.create_pipeline(gpu, SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT);
					cosmos.create_pipeline(gpu);
					atmosphere.create_pipeline(gpu);
				}
			}

			ImGui::End();

			ImGui::Render();

		}

		planet.update(dt);

		std::vector<SDL_GPUFence*> upload_fences;
		planet.upload(gpu, upload_fences);
		defer {
			for (SDL_GPUFence* upload_fence : upload_fences)
				SDL_ReleaseGPUFence(gpu, upload_fence);
		};

		SDL_GetMouseState(&mouse_x, &mouse_y);
		mouse_x /= targets.width;
		mouse_y /= targets.height;

		int num_keys;
		const bool* keys = SDL_GetKeyboardState(&num_keys);

		{
			f32 l = length(camera.position - camera.target);
			Vector3f d = normalize(camera.position - camera.target);
			l = l + (target_camera_distance - l) * (1 - std::expf(-10.f * dt));
			camera.position = camera.target + d * l;
		}

		arcball_camera.position = normalize(arcball_camera.position) * length(camera.position);

		if (keys[SDL_GetScancodeFromKey(SDLK_LSHIFT, nullptr)] && !io.WantCaptureKeyboard) {
			Vector3f d = normalize(camera.position - camera.target);
			target_camera_distance /= powf(1.3f, dt);
		}
		if (keys[SDL_GetScancodeFromKey(SDLK_LCTRL, nullptr)] && !io.WantCaptureKeyboard) {
			Vector3f d = normalize(camera.position - camera.target);
			target_camera_distance *= powf(1.3f, dt);
		}
		if (keys[SDL_GetScancodeFromKey(SDLK_F10, nullptr)] && !io.WantCaptureKeyboard) {
			SDL_SetWindowFullscreen(window, !is_fullscreen);
			is_fullscreen = !is_fullscreen;
			i32 w;
			i32 h;
			bool ok = SDL_GetWindowSizeInPixels(window, &w, &h);
			if (ok && w > 0 && h > 0) {
				targets.width = w;
				targets.height = h;
				if (!update_targets(gpu, targets)) {
					return 1;
				}
			} else {
				printf("Failed to get window size: %s\n", SDL_GetError());
			}
		}
		if (keys[SDL_GetScancodeFromKey(SDLK_Z, nullptr)] && !io.WantCaptureKeyboard) {
			f32 l = length(camera.target - camera.position);
			f32 l1 = std::abs(l - 1.0f);
			f32 s = l1 * l1 * PIf * camera.speed * dt;
			camera.position = camera.position + camera.up * s * dt;
			camera.position = normalize(camera.position) * l;
			camera.up = normalize(cross(cross(camera.position, camera.up), camera.position));
		}
		if (keys[SDL_GetScancodeFromKey(SDLK_S, nullptr)] && !io.WantCaptureKeyboard) {
			f32 l = length(camera.target - camera.position);
			f32 l1 = std::abs(l - 1.0f);
			f32 s = l1 * l1 * PIf * camera.speed * dt;
			camera.position = camera.position - camera.up * s * dt;
			camera.position = normalize(camera.position) * l;
			camera.up = normalize(cross(cross(camera.position, camera.up), camera.position));
		}
		if (keys[SDL_GetScancodeFromKey(SDLK_Q, nullptr)] && !io.WantCaptureKeyboard) {
			f32 l = length(camera.target - camera.position);
			f32 l1 = std::abs(l - 1.0f);
			f32 s = l1 * l1 * PIf * camera.speed * dt;
			Vector3f side = normalize(cross(camera.position, camera.up));
			camera.position = camera.position + side * s * dt;
			camera.position = normalize(camera.position) * l;
			camera.up = normalize(cross(cross(camera.position, camera.up), camera.position));
		}
		if (keys[SDL_GetScancodeFromKey(SDLK_D, nullptr)] && !io.WantCaptureKeyboard) {
			f32 l = length(camera.target - camera.position);
			f32 l1 = std::abs(l - 1.0f);
			f32 s = l1 * l1 * PIf * camera.speed * dt;
			Vector3f side = normalize(cross(camera.position, camera.up));
			camera.position = camera.position - side * s * dt;
			camera.position = normalize(camera.position) * l;
			camera.up = normalize(cross(cross(camera.position, camera.up), camera.position));
		}

		if (mouse_just_down[SDL_BUTTON_LEFT] && !io.WantCaptureMouse) {
			start_camera_pos = camera.position - camera.target;
			start_camera_up = camera.up;
			start_grab = arcball_camera.unproject_ray(mouse_x, mouse_y);
			start_grab = intersect_sphere_ray({ 0, 0, 0 }, 1, arcball_camera.position, *start_grab);
			start_grab_local = planet.mesh.local;
			grab_local_dt = identity();
			arcball_camera.position = camera.position;
			arcball_camera.up = camera.up;
		}
		if (mouse_down[SDL_BUTTON_LEFT] && start_grab.has_value()) {
			Vector3f fallback = arcball_camera.unproject_ray_ndc(0, 0);
			fallback = arcball_camera.unproject_ray(mouse_x, mouse_y) - fallback;
			fallback = normalize(fallback);

			end_grab = arcball_camera.unproject_ray(mouse_x, mouse_y);
			end_grab = intersect_sphere_ray(
				{ 0, 0, 0 }, 1, arcball_camera.position, end_grab
			).value_or(fallback);

			Vector3f axis = normalize(cross(*start_grab, end_grab));
			f32 angle = acosf(dot(*start_grab, end_grab));

			if (angle > 0.001f) {
				Quaternionf q = Quaternionf::axis_angle(axis, angle);
				grab_local_dt = to_rotation_matrix(q);
			}

			Vector3f dt = normalize(start_camera_pos - camera.target);
			dt = (Vector3f)(grab_local_dt * Vector4f(dt, 0));
			camera.position = camera.target + dt * length(camera.position - camera.target);
			camera.up = (Vector3f)(grab_local_dt * Vector4f(start_camera_up, 0.0f));
		}
		if (mouse_up[SDL_BUTTON_LEFT] && start_grab.has_value()) {
			start_grab = std::nullopt;
			grab_local_dt = identity();
			arcball_camera.position = camera.position;
			arcball_camera.up = camera.up;
		}

		SDL_GPUCommandBuffer* buffer = SDL_AcquireGPUCommandBuffer(gpu);
		SDL_GPUTexture* swapchain;
		if (!SDL_WaitAndAcquireGPUSwapchainTexture(buffer, window, &swapchain, nullptr, nullptr)) {
			printf("Failed to acquire swapchain texture: %s\n", SDL_GetError());
			return 1;
		}

		common_uniform.projection = perspective(camera.fov, 16.0f / 9.0f, 0.1f, 100.0f);
		common_uniform.view = lookAt(
			camera.position + planet.position, planet.position, camera.up
		);
		{
			SDL_GPUColorTargetInfo color_target = {};
			color_target.texture = targets.color_texture;
			color_target.clear_color = { 0.025f, 0.025f, 0.05f, 1.0f };
			color_target.load_op = SDL_GPU_LOADOP_DONT_CARE;
			color_target.store_op = SDL_GPU_STOREOP_STORE;
			color_target.resolve_texture = targets.resolve_texture;

			SDL_GPUDepthStencilTargetInfo depth_target = {};
			depth_target.texture = targets.depth_texture;
			depth_target.clear_depth = 1.0f;
			depth_target.load_op = SDL_GPU_LOADOP_CLEAR;
			depth_target.store_op = SDL_GPU_STOREOP_STORE;

			SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(
				buffer, &color_target, 1, &depth_target
			);

			cosmos.common_uniform = common_uniform;
			cosmos.render(pass, buffer);

			if (show_planet) {
				planet.common_uniform = common_uniform;
				planet.render(pass, buffer);
			}

			SDL_EndGPURenderPass(pass);

			if (show_planet) {
				color_target.load_op = SDL_GPU_LOADOP_LOAD;
				color_target.store_op = SDL_GPU_STOREOP_RESOLVE;
				pass = SDL_BeginGPURenderPass(buffer, &color_target, 1, nullptr);
				atmosphere.common_uniform = common_uniform;
				atmosphere.uniform.width = targets.width;
				atmosphere.uniform.height = targets.height;
				atmosphere.uniform.eye = camera.position;
				atmosphere.uniform.planet_world_position =
					(Vector3f)(planet.position);
				atmosphere.uniform.planet_radius = 1.0f;
				atmosphere.render(pass, buffer);
				SDL_EndGPURenderPass(pass);
			}
			
			SDL_GPUBlitInfo blit = {};
			blit.source.texture = targets.resolve_texture;
			blit.source.x = 0;
			blit.source.y = 0;
			blit.source.w = targets.width;
			blit.source.h = targets.height;
			blit.destination.texture = swapchain;
			blit.destination.x = 0;
			blit.destination.y = 0;
			blit.destination.w = targets.width;
			blit.destination.h = targets.height;
			blit.load_op = SDL_GPU_LOADOP_DONT_CARE;
			blit.filter = SDL_GPU_FILTER_LINEAR;
			if (blit.source.texture && blit.destination.texture)
				SDL_BlitGPUTexture(buffer, &blit);
		}

		// imgui
		if (true) {
			Imgui_ImplSDLGPU3_PrepareDrawData(ImGui::GetDrawData(), buffer);

			SDL_GPUColorTargetInfo target = {};
			target.texture = swapchain;
			target.clear_color = { 0.025f, 0.025f, 0.05f, 1.0f };
			target.load_op = SDL_GPU_LOADOP_LOAD;
			target.store_op = SDL_GPU_STOREOP_STORE;
			target.mip_level = 0;
			target.layer_or_depth_plane = 0;
			target.cycle = false;
			SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(buffer, &target, 1, nullptr);

			if (render_imgui)
				ImGui_ImplSDLGPU3_RenderDrawData(ImGui::GetDrawData(), buffer, pass);

			SDL_EndGPURenderPass(pass);
		}

		SDL_WaitForGPUFences(gpu, true, upload_fences.data(), upload_fences.size());
		if (!SDL_SubmitGPUCommandBuffer(buffer)) {
			printf("Failed to submit command buffer: %s\n", SDL_GetError());
			return 1;
		}
	}

	ImGui_ImplSDLGPU3_Shutdown();
	ImGui_ImplSDL3_Shutdown();

	return 0;
}

