#include "../include/utilities.hpp"
#include <iostream>
#include <fstream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <ktx.h>
#include "../include/constants.hpp"
#include "../include/vkFormat.hpp"

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

	void checkKtxError(ktx_error_code_e errorCode) {
		if (errorCode != ktx_error_code_e::KTX_SUCCESS) {
			throw std::runtime_error(ktxErrorString(errorCode));
		}
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

	wgpu::Texture getTexture(wgpu::Device& device, fastgltf::DataSource dataSource, std::string gltfDirectory)
	{
		if (!std::holds_alternative<fastgltf::sources::URI>(dataSource)) {
			throw std::runtime_error("Cannot get fastgltf::DataSource Texture, unsupported type");
		}
		fastgltf::sources::URI *p_uri = std::get_if<fastgltf::sources::URI>(&dataSource);
		if (p_uri->mimeType != fastgltf::MimeType::KTX2) {
			throw std::runtime_error("Only KTX2 Textures are supported");
		}
		
		ktxTexture2* p_ktxTexture;

		std::string filePath = gltfDirectory.append(p_uri->uri.c_str());
		checkKtxError(ktxTexture2_CreateFromNamedFile(filePath.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &p_ktxTexture));

		ktx_bool_t needsTranscoding = ktxTexture2_NeedsTranscoding(p_ktxTexture);
		std::cout << "needs transcoding: " << needsTranscoding << std::endl;

		ktx_transcode_fmt_e transcodeFormat = ktx_transcode_fmt_e::KTX_TTF_NOSELECTION;
		if (needsTranscoding) {
			//khr_df_model_e colorModel = ktxTexture2_GetColorModel_e(p_ktxTexture);
			throw std::runtime_error("ktx format unsupported");
			//checkKtxError(ktxTexture2_TranscodeBasis(p_ktxTexture, transcodeFormat, 0));
		}

		
		ktx_uint32_t numLevels = p_ktxTexture->numLevels;
		ktx_uint32_t baseWidth = p_ktxTexture->baseWidth;
		ktx_bool_t isArray = p_ktxTexture->isArray;
		std::cout << "numLevels: " << numLevels << std::endl;
		std::cout << "baseWidth: " << baseWidth << std::endl;
		std::cout << "isArray: " << isArray << std::endl;

		
		p_ktxTexture->vkFormat;
	//	result = ktxTexture_GetImageOffset(p_ktxTexture, level, layer, faceSlice, &offset);
		//ktxTexture2_GetColorModel_e() //DFD

	//	ktx_uint8_t* p_imageData = ktxTexture_GetData(p_ktxTexture) + offset;
	//	std::cout << "p_imageData: " << p_imageData << std::endl;
	//	ktxTexture2_Destroy(p_ktxTexture);
		
		wgpu::TextureDimension textureDimension = [&p_ktxTexture]() {
			switch (p_ktxTexture->numDimensions) {
			case 1:
				return wgpu::TextureDimension::e1D;
			case 2:
				return wgpu::TextureDimension::e2D;
			case 3:
				return wgpu::TextureDimension::e3D;
			default:
				throw std::runtime_error("unknown Texture Dimension");
			}
		}();

		wgpu::TextureDescriptor textureDescriptor = {
			.label = wgpu::StringView(filePath),
			.usage = wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopyDst,
			.dimension = textureDimension,
			.size = wgpu::Extent3D {
				.width = p_ktxTexture->baseWidth,
				.height = p_ktxTexture->baseHeight,
				.depthOrArrayLayers = p_ktxTexture->baseDepth,
			},
			.format = vkFormat::WebGpuImageFormat((vkFormat::VkFormat)p_ktxTexture->vkFormat),
			.mipLevelCount = p_ktxTexture->numLevels,
		};
		return wgpu::Texture();
	}

};
