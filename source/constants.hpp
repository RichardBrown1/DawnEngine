#pragma once
#include <webgpu/webgpu_cpp.h>
#include "structs.hpp"
namespace DawnEngine {
	constexpr wgpu::TextureFormat DEPTH_FORMAT = wgpu::TextureFormat::Depth16Unorm;
	constexpr glm::f32vec3 UP = { 0.0f, 1.0f, 0.0f };
	constexpr DawnEngine::Light DEFAULT_LIGHT = {
			.rotation = {2.755f, -0.286f, -1.269f}, //Points downwards and slightly in +X and +Z
			.color = {1.0f, 1.0f, 0.9f},
			.type = 0, //Directional
			.intensity = 128.0f,
	};
	constexpr DawnEngine::TextureInfo NO_TEXTURE = {
		.index = UINT32_MAX,
		.texCoord = 0,
	};
	//Corresponds to bitset textureOptions[i]
	enum TextureOptionsIndex {
		HAS_BASE_COLOR_TEXTURE = 0,
		HAS_METALLIC_ROUGHNESS_TEXTURE = 1,
	};

	//TODO: Fill this out with more Texture Types.
	enum TextureType {
		COLOR = 0,
		NORMAL = 1,
		METALLIC_ROUGHNESS = 2,
	};
}