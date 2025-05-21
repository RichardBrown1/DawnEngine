#pragma once
#include <string>
#include <ktx.h>
#include <webgpu/webgpu_cpp.h>
#include <absl/log/log.h>
#include "../wgpuContext/wgpuContext.hpp"	

namespace {
	void checkKtxError(ktx_error_code_e errorCode) {
		if (errorCode != ktx_error_code_e::KTX_SUCCESS) {
			LOG(FATAL) << ktxErrorString(errorCode);
		}
	}
}

namespace texture {
	namespace descriptor {
		struct CreateTextureView {
			std::string label;
			wgpu::Device* device;
			wgpu::Extent2D textureDimensions;
			wgpu::TextureFormat textureFormat;
			wgpu::TextureView& outputTextureView;
		};
	}

	void createTextureView(const descriptor::CreateTextureView* descriptor) {
		assert(descriptor->textureFormat != wgpu::TextureFormat::Undefined);
		const wgpu::TextureDescriptor textureDescriptor = {
					.label = wgpu::StringView(descriptor->label + std::string(" texture")),
					.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
					.dimension = wgpu::TextureDimension::e2D,
					.size = {
						.width = descriptor->textureDimensions.width,
						.height = descriptor->textureDimensions.height,
					},
					.format = descriptor->textureFormat,
		};
		wgpu::Texture texture = descriptor->device->CreateTexture(&textureDescriptor);
		const wgpu::TextureViewDescriptor textureViewDescriptor = {
			.label = wgpu::StringView(descriptor->label + std::string(" texture view")),
			.format = textureDescriptor.format,
			.dimension = wgpu::TextureViewDimension::e2D,
			.mipLevelCount = 1,
			.arrayLayerCount = 1,
			.aspect = wgpu::TextureAspect::All,
			.usage = textureDescriptor.usage,
		};
		descriptor->outputTextureView = texture.CreateView(&textureViewDescriptor);
	}

	void getTexture(WGPUContext& wgpuContext, std::string& filePath, wgpu::Texture& outTexture, wgpu::TextureView& outTextureView)
	{
		ktxTexture2* p_ktxTexture;
		LOG(INFO) << "Loading Texture: " << filePath;
		checkKtxError(ktxTexture2_CreateFromNamedFile(filePath.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &p_ktxTexture));
		std::shared_ptr<ktxTexture2> sp_ktxTexture2(p_ktxTexture, [](ktxTexture2* k2) {auto k = reinterpret_cast<ktxTexture*>(k2); ktxTexture_Destroy(k); });
		std::shared_ptr<ktxTexture> sp_ktxTexture = std::reinterpret_pointer_cast<ktxTexture, ktxTexture2>(sp_ktxTexture2);
		LOG(INFO) << "sp_ktxTexture2 Use Count: " << sp_ktxTexture2.use_count();

		ktx_bool_t needsTranscoding = ktxTexture2_NeedsTranscoding(sp_ktxTexture2.get());
		LOG(INFO) << "needs transcoding: " << needsTranscoding;

		ktx_transcode_fmt_e transcodeFormat = ktx_transcode_fmt_e::KTX_TTF_NOSELECTION;
		ktx_uint32_t numComponents = 0;
		ktx_uint32_t componentByteLength = 0;
		ktxTexture2_GetComponentInfo(sp_ktxTexture2.get(), &numComponents, &componentByteLength);
		ktx_uint32_t numChannels = 4; //magic number to replace
		LOG(INFO) << "numComponents: " << numComponents << " componentByteLength: " << componentByteLength;
		if (needsTranscoding) {
			khr_df_model_e colorModel = ktxTexture2_GetColorModel_e(sp_ktxTexture2.get());
			LOG(INFO) << transcodeFormat << wgpuContext.device.HasFeature(wgpu::FeatureName::TextureCompressionASTC);
			LOG(INFO) << colorModel;
			//throw std::runtime_error("ktx format unsupported");
			ktxTexture2_GetComponentInfo(sp_ktxTexture2.get(), &numComponents, &componentByteLength);
			LOG(INFO) << "numComponents: " << numComponents << " componentByteLength: " << componentByteLength;
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
				LOG(FATAL) << "unexpected numComponents() count";
			}
			checkKtxError(ktxTexture2_TranscodeBasis(sp_ktxTexture2.get(), transcodeFormat, 0));
		}

		needsTranscoding = ktxTexture2_NeedsTranscoding(sp_ktxTexture2.get());
		LOG(INFO) << "needs transcoding: " << needsTranscoding;

		ktx_size_t imageOffset;
		ktxTexture_GetImageOffset(sp_ktxTexture.get(), 0, 0, 0, &imageOffset);
		ktx_size_t imageSize = ktxTexture_GetImageSize(sp_ktxTexture.get(), 0);
		LOG(INFO) << "image size of level 0: " << imageSize;
		//sp_ktxTexture2->createMipmaps
		ktx_uint8_t* p8_textureData = ktxTexture_GetData(sp_ktxTexture.get());

		wgpu::TextureDescriptor textureDescriptor = {
			.label = wgpu::StringView(filePath),
			.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst,
			.dimension = wgpu::TextureDimension::e2D,
			.size = wgpu::Extent3D {
				.width = sp_ktxTexture->baseWidth,
				.height = sp_ktxTexture->baseHeight,
			},
			.format = wgpu::TextureFormat::BC7RGBAUnormSrgb,
			.mipLevelCount = 1,
		};
		outTexture = wgpuContext.device.CreateTexture(&textureDescriptor);

		const wgpu::TexelCopyTextureInfo texelCopyTextureInfo = {
			.texture = outTexture,
			.mipLevel = 0,
		};
		const wgpu::TexelCopyBufferLayout texelCopyBufferLayout = {
				.bytesPerRow = textureDescriptor.size.width * 4, //1 pixel = 4 bytes. Careful this will change with different formats
				.rowsPerImage = textureDescriptor.size.height,
		};

		wgpuContext.queue.WriteTexture(
			&texelCopyTextureInfo,
			p8_textureData,
			sizeof(float) * sp_ktxTexture->baseWidth * sp_ktxTexture->baseHeight,
			&texelCopyBufferLayout,
			&textureDescriptor.size
		);

		const wgpu::TextureViewDescriptor textureViewDescriptor = {
			.label = "Textures",
			.format = textureDescriptor.format,
			.dimension = wgpu::TextureViewDimension::e2D,
			.mipLevelCount = textureDescriptor.mipLevelCount,
			.arrayLayerCount = textureDescriptor.size.depthOrArrayLayers,
			.usage = textureDescriptor.usage,
		};

		outTextureView = outTexture.CreateView(&textureViewDescriptor);
	}
}
