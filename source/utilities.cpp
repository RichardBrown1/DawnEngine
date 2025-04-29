#include "utilities.hpp"
#include <iostream>
#include <fstream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <ktx.h>
#include "constants.hpp"
#include "vkFormat.hpp"

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

	void checkKtxError(ktx_error_code_e errorCode) {
		if (errorCode != ktx_error_code_e::KTX_SUCCESS) {
			throw std::runtime_error(ktxErrorString(errorCode));
		}
	}
};

namespace Utilities {

	void unreachable()
	{
		// Uses compiler specific extensions if possible.
		// Even if no extension is used, undefined behavior is still raised by
		// an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
		__assume(false);
#else // GCC, Clang
		__builtin_unreachable();
#endif
	}

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

	wgpu::AddressMode convertType(fastgltf::Wrap wrap) {
		switch (wrap) {
		case fastgltf::Wrap::ClampToEdge:
			return wgpu::AddressMode::ClampToEdge;
		case fastgltf::Wrap::MirroredRepeat:
			return wgpu::AddressMode::MirrorRepeat;
		case fastgltf::Wrap::Repeat:
			return wgpu::AddressMode::Repeat;
		default:
			throw std::runtime_error("Invalid wrap conversion");
		}
	};

	wgpu::FilterMode convertFilter(fastgltf::Filter filter) {
		switch (filter) {
		case fastgltf::Filter::Linear:
		case fastgltf::Filter::LinearMipMapLinear:
		case fastgltf::Filter::LinearMipMapNearest:
			return wgpu::FilterMode::Linear;
		case fastgltf::Filter::Nearest:
		case fastgltf::Filter::NearestMipMapLinear:
		case fastgltf::Filter::NearestMipMapNearest:
			return wgpu::FilterMode::Nearest;
		default:
			throw std::runtime_error("Invalid filter conversion");
		}
	};

	wgpu::MipmapFilterMode convertMipMapFilter(fastgltf::Filter filter) {
		switch (filter) {
		case fastgltf::Filter::Linear:
		case fastgltf::Filter::Nearest:
			return wgpu::MipmapFilterMode::Undefined;
		case fastgltf::Filter::LinearMipMapLinear:
		case fastgltf::Filter::NearestMipMapLinear:
			return wgpu::MipmapFilterMode::Linear;
		case fastgltf::Filter::LinearMipMapNearest:
		case fastgltf::Filter::NearestMipMapNearest:
			return wgpu::MipmapFilterMode::Nearest;
		default:
			throw std::runtime_error("Invalid mipmap filter conversion");
		}
	};

	wgpu::ShaderModule Utilities::createShaderModule(wgpu::Device& device, const wgpu::StringView& label, const std::string& filename)
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
};

namespace DawnEngine {

	DawnEngine::H_Camera getDefaultCamera(wgpu::SurfaceConfiguration surfaceConfiguration)
	{
		return {
			.projection = glm::perspectiveRH_ZO(45.0f, surfaceConfiguration.width / (float)surfaceConfiguration.height, 0.00001f, 1024.0f),
			.position = { 0.0f, 0.0f, 0.1f },
			.forward = { 0.0f, 0.0f, 0.1f },
		};
	}

	DawnEngine::TextureInfo convertType(const fastgltf::TextureInfo &textureInfo)
	{
			DawnEngine::TextureInfo dawnEngineTextureInfo = {
				.index = static_cast<uint32_t>(textureInfo.textureIndex),
				.texCoord = static_cast<uint32_t>(textureInfo.texCoordIndex),
			};
			return dawnEngineTextureInfo;
	}

};
