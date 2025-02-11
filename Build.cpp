
#include "Ease.hpp"

EASE_WATCH_ME;

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

Build build(Flags flags) noexcept {
	flags.no_install_path = true;
	compile_shaders();
	
	Build b = Build::get_default(flags);
	b.flags.subsystem = Flags::Subsystem::Console;
	b.flags.disable_exceptions = true;
	b.flags.compile_native = true;
	b.flags.generate_debug = true;
	b.flags.allow_temporary_address = true;

	b.name = "place";

	b.add_header("src/");
	b.add_header("include/");
	b.add_source_recursively("src/");
	b.add_library_path("lib/");

	b.add_define("WIN32_LEAN_AND_MEAN");
	b.add_define("NOMINMAX");

	b.add_library("kernel32.lib");
	b.add_library("SDL3.lib");

	return b;
}