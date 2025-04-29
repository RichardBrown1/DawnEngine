#pragma once
#include <iostream>
#include <ktx.h>
#include "textureSamplerManager.hpp"
#include "utilities.hpp"

namespace {
	void checkKtxError(ktx_error_code_e errorCode) {
		if (errorCode != ktx_error_code_e::KTX_SUCCESS) {
			throw std::runtime_error(ktxErrorString(errorCode));
		}
	}

	struct TextureInputInfo {
		wgpu::Extent2D dimensions;
		uint32_t textureSamplerPairId;
		uint32_t PAD0;
	};

}

namespace DawnEngine {
	TextureSamplerManager::TextureSamplerManager(const TextureSamplerManagerDescriptor* descriptor) {
		_device = descriptor->device;
		_accumulatorTextureDimensions = descriptor->accumulatorTextureDimensions;
		_invocationSize = descriptor->invocationSize;
		_textureIndicesMap = descriptor->textureIndicesMap;

		_queue = _device.GetQueue();

		_baseColorAccumulatorShaderModule = Utilities::createShaderModule(
			_device,
			BASE_COLOR_ACCUMULATOR_SHADER_LABEL,
			BASE_COLOR_ACCUMULATOR_SHADER_PATH
		);
		baseColorAccumulatorTextureFormat = descriptor->baseColorAccumulatorTextureFormat;
	};

	void TextureSamplerManager::addAsset(fastgltf::Asset& asset, std::string gltfDirectory) {
		for (uint32_t i = 0; auto & t : asset.textures) {
			addSamplerTexturePair(t, _textureIndicesMap.at(i));
			addTextureInputInfoBuffer(i);
			++i;
		}
		for (auto& i : asset.images) {
			addTexture(i.data, gltfDirectory);
		}
		for (auto& s : asset.samplers) {
			addSampler(s);
		}
	}

	void TextureSamplerManager::addSamplerTexturePair(fastgltf::Texture texture, DawnEngine::TextureType textureType) {
		if (!texture.basisuImageIndex.has_value()) {
			throw std::runtime_error("only KTX files are able to be used as textures");
		}
		const DawnEngine::SamplerTexturePair samplerTexturePair = {
			.samplerIndex = static_cast<uint32_t>(texture.samplerIndex.has_value()),
			.textureIndex = static_cast<uint32_t>(texture.basisuImageIndex.has_value()),
			.textureType = textureType,
		};
		_samplerTexturePairs.push_back(samplerTexturePair);
	}

	void TextureSamplerManager::addTextureInputInfoBuffer(uint32_t samplerTexturePairIndex) {
		const TextureInputInfo textureInputInfo = {
			.dimensions = _accumulatorTextureDimensions,
			.textureSamplerPairId = samplerTexturePairIndex,
		};
		wgpu::BufferDescriptor textureInputInfoBufferDescriptor = {
			.label = "texture input buffer descriptor",
			.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst,
			.size = sizeof(TextureInputInfo),
		};
		wgpu::Buffer textureInputInfoBuffer = _device.CreateBuffer(&textureInputInfoBufferDescriptor);
		_queue.WriteBuffer(textureInputInfoBuffer, 0, &textureInputInfo, textureInputInfoBufferDescriptor.size);
		_textureInputInfoBuffers.push_back(textureInputInfoBuffer);
	}

	void TextureSamplerManager::addTexture(fastgltf::DataSource dataSource, std::string gltfDirectory)
	{
		if (!std::holds_alternative<fastgltf::sources::URI>(dataSource)) {
			throw std::runtime_error("Cannot get fastgltf::DataSource Texture, unsupported type");
		}
		fastgltf::sources::URI* p_uri = std::get_if<fastgltf::sources::URI>(&dataSource);
		if (p_uri->mimeType != fastgltf::MimeType::KTX2) {
			throw std::runtime_error("Only KTX2 Textures are supported");
		}

		ktxTexture2* p_ktxTexture;
		std::string filePath = gltfDirectory.append(p_uri->uri.c_str());
		std::cout << "Loading Texture: " << filePath << std::endl;
		checkKtxError(ktxTexture2_CreateFromNamedFile(filePath.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &p_ktxTexture));
		std::shared_ptr<ktxTexture2> sp_ktxTexture2(p_ktxTexture, [](ktxTexture2* k2) {auto k = reinterpret_cast<ktxTexture*>(k2); ktxTexture_Destroy(k); });
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
			std::cout << transcodeFormat << _device.HasFeature(wgpu::FeatureName::TextureCompressionASTC);
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
		wgpu::Texture texture = _device.CreateTexture(&textureDescriptor);
		_textures.push_back(texture);

		const wgpu::TexelCopyTextureInfo texelCopyTextureInfo = {
			.texture = texture,
			.mipLevel = 0,
		};
		const wgpu::TexelCopyBufferLayout texelCopyBufferLayout = {
				.bytesPerRow = textureDescriptor.size.width * 4, //1 pixel = 4 bytes. Careful this will change with different formats
				.rowsPerImage = textureDescriptor.size.height,
		};

		_queue.WriteTexture(
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
		_textureViews.push_back(texture.CreateView(&textureViewDescriptor));
	}

	void TextureSamplerManager::addSampler(fastgltf::Sampler sampler) {
		wgpu::SamplerDescriptor	samplerDescriptor = {
				.label = sampler.name.c_str(),
				.addressModeU = Utilities::convertType(sampler.wrapS),
				.addressModeV = Utilities::convertType(sampler.wrapT),
				.magFilter = Utilities::convertFilter(sampler.magFilter.value_or(fastgltf::Filter::Linear)),
				.minFilter = Utilities::convertFilter(sampler.minFilter.value_or(fastgltf::Filter::Linear)),
				.mipmapFilter = Utilities::convertMipMapFilter(sampler.minFilter.value_or(fastgltf::Filter::Linear)),
		};
		_samplers.push_back(_device.CreateSampler(&samplerDescriptor));
	}

	//create Texture Pipeline before this
	void TextureSamplerManager::doTextureCommands(const DoTextureCommandsBindGroupDescriptor* descriptor) {
		wgpu::ComputePassDescriptor computePassDescriptor = {
			.label = "texture compute pass",
		};
		wgpu::ComputePassEncoder computePassEncoder = descriptor->commandEncoder.BeginComputePass(&computePassDescriptor);
		computePassEncoder.SetPipeline(_computePipeline);
		CreateAccumulatorAndInfoBindGroupDescriptor createAccumulatorAndInfoBindGroupDescriptor = {
			.accumulatorTextureView = descriptor->accumulatorTextureView,
			.infoBuffer = descriptor->infoBuffer,
		};
		const wgpu::BindGroup accumulatorAndInfoBindGroup = createAccumulatorAndInfoBindGroup(&createAccumulatorAndInfoBindGroupDescriptor);
		computePassEncoder.SetBindGroup(0, accumulatorAndInfoBindGroup);

		for (int i = 0; auto & stp : _samplerTexturePairs) {
			//TODO: _samplerTexturePairs need to be split into their own arrays on generation
			if (stp.textureType != DawnEngine::TextureType::COLOR) {
				continue;
			}
			CreateInputTextureBindGroupDescriptor createInputTextureBindGroupDescriptor = {
				.textureInputInfoBuffer = _textureInputInfoBuffers[i],
				.inputTexture = _textureViews[stp.samplerIndex]
			};
			const wgpu::BindGroup inputTextureBindGroup = createInputTextureBindGroup(&createInputTextureBindGroupDescriptor);
			computePassEncoder.SetBindGroup(1, inputTextureBindGroup);
			computePassEncoder.DispatchWorkgroups(_accumulatorTextureDimensions.width, _accumulatorTextureDimensions.height);

			++i;
		}
		computePassEncoder.End();
	}


	wgpu::ComputePipeline TextureSamplerManager::createTexturePipeline(const CreateTexturePipelineDescriptor* descriptor) {
		const wgpu::ComputeState computeState = {
			.module = _baseColorAccumulatorShaderModule,
			.entryPoint = "CS_Main"
		};
		const std::array<wgpu::BindGroupLayout, 2> bindGroupLayouts = {
			getAccumulatorAndInfoBindGroupLayout(descriptor->colorTextureFormat),
			getInputBindGroupLayout(),
		};

		const wgpu::PipelineLayoutDescriptor computePipelineLayoutDescriptor = {
			.label = "texture pipeline layout",
			.bindGroupLayoutCount = bindGroupLayouts.size(),
			.bindGroupLayouts = bindGroupLayouts.data(),
		};
		const wgpu::PipelineLayout computePipelineLayout = _device.CreatePipelineLayout(&computePipelineLayoutDescriptor);
		const wgpu::ComputePipelineDescriptor computePipelineDescriptor = {
			.label = "compute pipeline",
			.layout = computePipelineLayout,
			.compute = computeState,
		};
		return _device.CreateComputePipeline(&computePipelineDescriptor);
	}

	wgpu::BindGroupLayout TextureSamplerManager::getAccumulatorAndInfoBindGroupLayout(wgpu::TextureFormat accumulatorTextureFormat) {
		//I think no samplers are needed for the accumulator and the info since we can get the pixel and the textures should be screen size
		wgpu::BindGroupLayoutEntry accumulatorTexture = {
			.binding = 0,
			.storageTexture = {
				.access = wgpu::StorageTextureAccess::ReadWrite, //TODO: Should this be write only if I am only doing partial writes?
				.format = accumulatorTextureFormat,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};
		wgpu::BindGroupLayoutEntry infoBuffer = { //this will have the same amount of elements as the screen size
			.binding = 1,
			.buffer = {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(DawnEngine::TextureMasterInfo),
			},
		};
		std::array<wgpu::BindGroupLayoutEntry, 2>  bindGroupLayoutEntries = {
			accumulatorTexture,
			infoBuffer,
		};
		wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "texture accumulator and info bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		return _device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);
	}

	wgpu::BindGroupLayout TextureSamplerManager::getInputBindGroupLayout() {
		wgpu::BindGroupLayoutEntry textureInputInfoBuffer = {
			.binding = 0,
			.buffer = {
				.type = wgpu::BufferBindingType::Uniform,
				.minBindingSize = sizeof(TextureInputInfo),
			},
		};
		wgpu::BindGroupLayoutEntry inputTexture = {
			.binding = 1,
			.texture = {
				.sampleType = wgpu::TextureSampleType::Float,
				.viewDimension = wgpu::TextureViewDimension::e2D,
			},
		};
		wgpu::BindGroupLayoutEntry inputSampler = {
			.binding = 2,
			.sampler = {
				.type = wgpu::SamplerBindingType::Filtering, //Use case of non filtering samplers?
			},
		};
		std::array<wgpu::BindGroupLayoutEntry, 3>  bindGroupLayoutEntries = {
			textureInputInfoBuffer,
			inputTexture,
			inputSampler,
		};
		wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
			.label = "texture input bind group layout",
			.entryCount = bindGroupLayoutEntries.size(),
			.entries = bindGroupLayoutEntries.data(),
		};
		return _device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);
	}

	wgpu::BindGroup TextureSamplerManager::createAccumulatorAndInfoBindGroup(const CreateAccumulatorAndInfoBindGroupDescriptor* descriptor) {
		const wgpu::BindGroupEntry accumulatorTexture = {
			.textureView = descriptor->accumulatorTextureView,
		};
		const wgpu::BindGroupEntry infoBuffer = {
			.buffer = descriptor->infoBuffer,
		};
		const std::array<wgpu::BindGroupEntry, 2> bindGroupEntries = {
				accumulatorTexture,
				infoBuffer,
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "accumulator and info bind group",
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		return _device.CreateBindGroup(&bindGroupDescriptor);
	}

	wgpu::BindGroup TextureSamplerManager::createInputTextureBindGroup(const CreateInputTextureBindGroupDescriptor* descriptor) {
		const wgpu::BindGroupEntry textureInputInfoBuffer = {
			.buffer = descriptor->textureInputInfoBuffer,
		};
		const wgpu::BindGroupEntry inputTexture = {
			.textureView = descriptor->inputTexture,
		};
		const wgpu::BindGroupEntry inputSampler = {
			.sampler = descriptor->inputSampler,
		};
		const std::array<wgpu::BindGroupEntry, 3> bindGroupEntries = {
				textureInputInfoBuffer,
				inputTexture,
				inputSampler,
		};
		const wgpu::BindGroupDescriptor bindGroupDescriptor = {
			.label = "accumulator and info bind group",
			.entryCount = bindGroupEntries.size(),
			.entries = bindGroupEntries.data(),
		};
		return _device.CreateBindGroup(&bindGroupDescriptor);
	}
}
