#pragma once
#include <dawn/webgpu_cpp.h>
#include <string>
#include <vector>
#include "../wgpuContext/wgpuContext.hpp"

namespace device {
	struct SceneResources {
		wgpu::Buffer vbo;
		wgpu::Buffer transforms;
		wgpu::Buffer indices;
		wgpu::Buffer materialIndices; //MaterialId for each instance

		std::vector<wgpu::Buffer> lights;
		wgpu::Buffer cameras;

		wgpu::Buffer materials;
		wgpu::Buffer samplerTexturePairs;

		std::unordered_map<uint32_t, wgpu::Sampler> samplers;
		std::vector<wgpu::Texture> textures;
		std::vector<wgpu::TextureView> textureViews;
	};

	wgpu::ShaderModule createShaderModule(
		const wgpu::Device& device,
		const wgpu::StringView& label,
		const std::string& filename
	);

	wgpu::ShaderModule createWGSLShaderModule(
		const wgpu::Device& device,
		const wgpu::StringView& label,
		const std::string& filename
	);
}