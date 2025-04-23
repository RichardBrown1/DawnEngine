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
};

namespace DawnEngine {
	wgpu::ShaderModule	createShaderModule(wgpu::Device& device, const wgpu::StringView& label, const std::string& filename);
	DawnEngine::H_Camera getDefaultCamera(wgpu::SurfaceConfiguration surfaceConfiguration);

	void convertType(std::optional<fastgltf::TextureInfo>& fastGltfTextureInfo, DawnEngine::TextureInfo& DawnEngineTextureInfo);
	void getTexture(wgpu::Device& device, fastgltf::DataSource dataSource, std::string gltfDirectory, std::array<std::array<uint32_t, 2048>, 2048>& hostTexture);
};
