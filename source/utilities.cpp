#include "../include/utilities.hpp"
#include <iostream>
#include <fstream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <ktx.h>
#include "../include/constants.hpp"

namespace Utilities {

	void checkFastGltfError(const fastgltf::Error& error, const std::string& additionalMessage) {
		if (error == fastgltf::Error::None) {
			return;
		}
		throw std::runtime_error(
			std::format(" \
				fastgltf error name: {} \r\n \
				additionalMessage: {} \r\n \
				errorMessage: {} \r\n",
				getErrorName(error),
				additionalMessage,
				getErrorMessage(error)
			)
		);
	};
};


namespace {
	std::vector<uint32_t> readShader(const std::string& filename) {
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		if (!file.is_open()) {
			throw std::runtime_error("Failed to open SPIR-V Shader file.");
		}

		size_t fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
		file.close();

		return buffer;
	}
};

namespace DawnEngine {
	wgpu::ShaderModule DawnEngine::createShaderModule(wgpu::Device& device, const wgpu::StringView& label, const std::string& filename)
	{
		const std::vector<uint32_t> shaderCode = readShader(std::string(filename));
		wgpu::ShaderSourceSPIRV shaderSource = wgpu::ShaderSourceSPIRV();
		shaderSource.codeSize = static_cast<uint32_t>(shaderCode.size());
		shaderSource.code = shaderCode.data();
		const wgpu::ShaderModuleDescriptor shaderModuleDescriptor = {
			.nextInChain = &shaderSource,
			.label = label,
		};
		return device.CreateShaderModule(&shaderModuleDescriptor);
	}

	DawnEngine::Camera getDefaultCamera(wgpu::SurfaceConfiguration surfaceConfiguration)
	{
		constexpr glm::f32vec3 eye = { 0.0f, 0.0f, -10.0f };
		constexpr glm::f32vec3 origin = glm::f32vec3(0.0f, 0.0f, 0.0f);
		constexpr glm::f32vec3 up = glm::f32vec3(0.0, 1.0f, 0.0f);
		return DawnEngine::Camera{
			.projection = glm::perspectiveRH_ZO(45.0f, surfaceConfiguration.width / (float)surfaceConfiguration.height, 0.1f, 1024.0f),
			.view = glm::lookAt(eye, origin, up),
		};
	}

	void convertType(std::optional<fastgltf::TextureInfo> &fastgltfTextureInfo, DawnEngine::TextureInfo& dawnEngineTextureInfo)
	{
		if (fastgltfTextureInfo.has_value()) {
			fastgltf::TextureInfo &ti = fastgltfTextureInfo.value();
			dawnEngineTextureInfo = {
				.index = static_cast<uint32_t>(ti.textureIndex),
				.texCoord = static_cast<uint32_t>(ti.texCoordIndex),
			};
		}	else {
			dawnEngineTextureInfo = DawnEngine::NO_TEXTURE;
		}
	}

	wgpu::Texture getTexture(fastgltf::DataSource dataSource)
	{
		if (!std::holds_alternative<fastgltf::sources::URI>(dataSource)) {
			throw std::runtime_error("Cannot get fastgltf::DataSource Texture, unsupported type");
		}
		fastgltf::sources::URI *p_uri = std::get_if<fastgltf::sources::URI>(&dataSource);
		if (p_uri->mimeType != fastgltf::MimeType::KTX2) {
			throw std::runtime_error("Only KTX2 Textures are supported");
		}
		
		ktxTexture* p_ktxTexture;
		KTX_error_code result;

		result = ktxTexture_CreateFromNamedFile(p_uri->uri.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &p_ktxTexture);
		if (result != ktx_error_code_e::KTX_SUCCESS) {
			throw std::runtime_error(ktxErrorString(result));
		}
		
		ktxTexture_Destroy(p_ktxTexture);
		return wgpu::Texture();
	}

};
