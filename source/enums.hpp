#pragma once
#include <dawn/webgpu_cpp.h>

namespace enums {

	namespace EntryPoint { //fake enum - is there a better way to do this?
		constexpr wgpu::StringView VERTEX = "vs_main";
		constexpr wgpu::StringView FRAGMENT = "fs_main";
		constexpr wgpu::StringView COMPUTE = "cs_main";
	};

	//Corresponds to bitset textureOptions[i]
	enum TextureOptionsIndex {
		HAS_BASE_COLOR_TEXTURE = 0,
		HAS_METALLIC_ROUGHNESS_TEXTURE = 1,
	};

	//TODO: Fill this out with more Texture Types.
	enum class MaterialProperty {
		COLOR = 0,
		NORMAL = 1,
		METALLIC_ROUGHNESS = 2,
	};

};