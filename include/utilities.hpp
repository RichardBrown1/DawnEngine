#pragma once
#include <vector>
#include <string>
#include <webgpu/webgpu_cpp.h>
#include <fastgltf/core.hpp>
#include <glm/glm.hpp>
#include "structs.hpp"

namespace Utilities {
	void checkFastGltfError(const fastgltf::Error& error, const std::string& additionalMessage = "");
};

namespace DawnEngine {
	wgpu::ShaderModule	createShaderModule(wgpu::Device& device, const wgpu::StringView& label, const std::string& filename);
	DawnEngine::Camera getDefaultCamera(wgpu::SurfaceConfiguration surfaceConfiguration);
	
	void convertType(std::optional<fastgltf::TextureInfo>& fastGltfTextureInfo, DawnEngine::TextureInfo& DawnEngineTextureInfo);
	wgpu::Texture getTexture(fastgltf::DataSource dataSource);
};
