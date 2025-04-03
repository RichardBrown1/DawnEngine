#pragma once
#include <webgpu/webgpu_cpp.h>
#include "structs.hpp"
namespace DawnEngine {
	const wgpu::TextureFormat DEPTH_FORMAT = wgpu::TextureFormat::Depth16Unorm;
	
	const DawnEngine::Light DEFAULT_LIGHT = {
		//.rotation = {0, }
		.color = {1.0, 1.0, 0.8},

	};
}