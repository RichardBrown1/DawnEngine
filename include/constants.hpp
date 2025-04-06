#pragma once
#include <webgpu/webgpu_cpp.h>
#include "structs.hpp"
namespace DawnEngine {
	constexpr wgpu::TextureFormat DEPTH_FORMAT = wgpu::TextureFormat::Depth16Unorm;
	constexpr DawnEngine::Light DEFAULT_LIGHT = {
			.rotation = {2.755f, -0.286f, -1.269f}, //Points downwards and slightly in +X and +Z
			.color = {1.0f, 1.0f, 0.9f},
			.type = 0, //Directional
			.intensity = 128.0f,
	};

}