#include "utilities.hpp"
#include <iostream>
#include <fstream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <ktx.h>
#include "constants.hpp"
#include "vkFormat.hpp"

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
		constexpr glm::f32vec3 eye = { 0.0f, 0.0f, -0.1f };
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

	void getTexture(wgpu::Device& device, fastgltf::DataSource dataSource, std::string gltfDirectory, std::array<std::array<uint32_t, 2048>, 2048> &hostTexture)
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
		std::cout << "Loading Texture: " << filePath << std::endl;
		checkKtxError(ktxTexture2_CreateFromNamedFile(filePath.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &p_ktxTexture));
		std::shared_ptr<ktxTexture2> sp_ktxTexture2(p_ktxTexture, [](ktxTexture2* k2){auto k = reinterpret_cast<ktxTexture*>(k2); ktxTexture_Destroy(k); });
		std::shared_ptr<ktxTexture> sp_ktxTexture = std::reinterpret_pointer_cast<ktxTexture, ktxTexture2>(sp_ktxTexture2);
		std::cout << "sp_ktxTexture2 Use Count: " << sp_ktxTexture2.use_count() << std::endl;

		ktx_bool_t needsTranscoding = ktxTexture2_NeedsTranscoding(sp_ktxTexture2.get());
		std::cout << "needs transcoding: " << needsTranscoding << std::endl;

		ktx_transcode_fmt_e transcodeFormat = ktx_transcode_fmt_e::KTX_TTF_NOSELECTION;
		ktx_uint32_t numComponents = 0;
		ktx_uint32_t componentByteLength = 0;
		ktxTexture2_GetComponentInfo(sp_ktxTexture2.get(), &numComponents, &componentByteLength);
		ktx_uint32_t numChannels = 4; //magic number to replace
		std::cout << "numComponents: " << numComponents << " componentByteLength: " << componentByteLength << std::endl;
		if (needsTranscoding) {
			khr_df_model_e colorModel = ktxTexture2_GetColorModel_e(sp_ktxTexture2.get());
			std::cout << transcodeFormat << device.HasFeature(wgpu::FeatureName::TextureCompressionASTC);
			std::cout << colorModel << std::endl;
			//throw std::runtime_error("ktx format unsupported");
			ktxTexture2_GetComponentInfo(sp_ktxTexture2.get(), &numComponents, &componentByteLength);
			std::cout << "numComponents: " << numComponents << " componentByteLength: " << componentByteLength << std::endl;
				//TODO Implement other BC Formats.
				//5 and 6H seem interesting
			switch (numChannels) {
			case 1:
				transcodeFormat = ktx_transcode_fmt_e::KTX_TTF_BC4_R;
				break;
			case 2:
				transcodeFormat = ktx_transcode_fmt_e::KTX_TTF_BC5_RG;
				break;
			case 3:
			case 4:
				transcodeFormat = ktx_transcode_fmt_e::KTX_TTF_BC7_RGBA;
				break;
			default:
				throw std::runtime_error("unexpected numComponents() count");
			}
			checkKtxError(ktxTexture2_TranscodeBasis(sp_ktxTexture2.get(), transcodeFormat, 0));
		}
		
		needsTranscoding = ktxTexture2_NeedsTranscoding(sp_ktxTexture2.get());
		std::cout << "needs transcoding: " << needsTranscoding << std::endl;

		ktx_size_t imageOffset;
		ktxTexture_GetImageOffset(sp_ktxTexture.get(), 0, 0, 0, &imageOffset);
		ktx_size_t imageSize = ktxTexture_GetImageSize(sp_ktxTexture.get(), 0);
		std::cout << "image size of level 0: " << imageSize << std::endl;
		//sp_ktxTexture2->generateMipmaps
		ktx_uint8_t *p8_textureData = ktxTexture_GetData(sp_ktxTexture.get());
		memcpy(hostTexture.data(), p8_textureData + imageOffset, imageSize);

//		wgpu::BufferDescriptor stagingBufferDescriptor = {
//			.label = "texture staging buffer",
//			.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc,
//			.size = ktxTexture_GetImageSize(sp_ktxTexture.get(), 0),
//			.mappedAtCreation = true,
//		};
//		wgpu::Buffer stagingBuffer = device.CreateBuffer(&stagingBufferDescriptor);
//		//wgpu::TextureDataLayout().
	}

};
