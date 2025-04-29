#pragma once
#include <vector>
#include <string>
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/core.hpp>
#include <glm/glm.hpp>
#include "structs.hpp"

namespace Utilities {
	void unreachable();
	void checkFastGltfError(const fastgltf::Error& error, const std::string& additionalMessage = "");
	wgpu::AddressMode convertType(fastgltf::Wrap wrap);
	wgpu::FilterMode convertFilter(fastgltf::Filter filter);
	wgpu::MipmapFilterMode convertMipMapFilter(fastgltf::Filter filter);
	wgpu::ShaderModule createShaderModule(wgpu::Device& device, const wgpu::StringView& label, const std::string& filename);
};

namespace DawnEngine {
	DawnEngine::H_Camera getDefaultCamera(wgpu::SurfaceConfiguration surfaceConfiguration);
	DawnEngine::TextureInfo convertType(const fastgltf::TextureInfo& textureInfo);
	void getTexture(wgpu::Device& device, fastgltf::DataSource dataSource, std::string gltfDirectory, std::array<std::array<uint32_t, 2048>, 2048>& hostTexture);
};
