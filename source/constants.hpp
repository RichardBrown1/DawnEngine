#pragma once
#include <glm/glm.hpp>
#include <dawn/webgpu_cpp.h>

namespace constants {
	constexpr glm::f32vec3 UP = glm::f32vec3{ 0.0f, 1.0f, 0.0f };
	constexpr wgpu::TextureFormat DEPTH_FORMAT = wgpu::TextureFormat::Depth32Float;
}