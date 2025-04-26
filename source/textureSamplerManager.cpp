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
}

//TODO - Create a render pipeline that will do each Texture Sampling in a compute shader (can i just loop this in draw call?)
//TODO - Create binding and binding group generator 

TextureSamplerManager::TextureSamplerManager(const TextureSamplerManagerDescriptor &descriptor) {
	_device = descriptor.device;
	_workgroupSize = descriptor.workgroupSize;
	_invocationSize = descriptor.invocationSize;
	_textureIndicesMap = descriptor.textureIndicesMap;

	_baseColorAccumulatorShaderModule = Utilities::createShaderModule(
		_device,
		BASE_COLOR_ACCUMULATOR_SHADER_LABEL,
		BASE_COLOR_ACCUMULATOR_SHADER_PATH
	);
};

void TextureSamplerManager::addAsset(fastgltf::Asset& asset, std::string gltfDirectory) {
	for (uint32_t i = 0; auto & t : asset.textures) {
		addTextureSamplerPair(t, _textureIndicesMap.at(i));
		++i;
	}
	for (auto &i : asset.images) {
		addTexture(i.data, gltfDirectory);
	}
	for (auto& s : asset.samplers) {
		addSampler(s);
	}
}

void TextureSamplerManager::addTextureSamplerPair(fastgltf::Texture texture, DawnEngine::TextureType textureType) {
		if (!texture.basisuImageIndex.has_value()) {
			throw std::runtime_error("only KTX files are able to be used as textures");
		}
		const DawnEngine::SamplerTexturePair samplerTexturePair = {
			.samplerIndex = static_cast<uint32_t>(texture.samplerIndex.has_value()),
			.textureIndex = static_cast<uint32_t>(texture.basisuImageIndex.has_value()),
			.textureType = textureType,
		};
		_samplerTexturePair.push_back(samplerTexturePair);
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
	//sp_ktxTexture2->generateMipmaps
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

	const wgpu::Queue queue = _device.GetQueue();
	queue.WriteTexture(
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

//generate Texture Pipeline before this
void TextureSamplerManager::doTextureCommands(wgpu::CommandEncoder& commandEncoder) {
	wgpu::ComputePassDescriptor computePassDescriptor = {
		.label = "texture compute pass",
	};
	wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass(&computePassDescriptor);
	computePassEncoder.SetPipeline(_computePipeline);
	computePassEncoder.DispatchWorkgroups(_workgroupSize.width, _workgroupSize.height);
	computePassEncoder.End();
}


wgpu::ComputePipeline TextureSamplerManager::generateTexturePipeline(GenerateTexturePipelineDescriptor descriptor) {
	wgpu::ComputeState computeState = {
		.module = _baseColorAccumulatorShaderModule,
		.entryPoint = "CS_Main"
	};
	const wgpu::BindGroupLayout bindGroupLayout = getBindGroupLayout(descriptor.colorTextureFormat);

	const wgpu::PipelineLayoutDescriptor computePipelineLayoutDescriptor = {
		.label = "texture pipeline layout",
		.bindGroupLayoutCount = 1,
		.bindGroupLayouts = &bindGroupLayout,
	};
	wgpu::PipelineLayout computePipelineLayout = _device.CreatePipelineLayout(&computePipelineLayoutDescriptor);
	wgpu::ComputePipelineDescriptor computePipelineDescriptor = {
		.label = "compute pipeline",
		.layout = computePipelineLayout,
		.compute = computeState,
	};
}

wgpu::BindGroupLayout TextureSamplerManager::getBindGroupLayout(wgpu::TextureFormat accumulatorTextureFormat) {
	//I think no samplers are needed for the accumulator and the info since we can get the pixel and the textures should be screen size
	wgpu::BindGroupLayoutEntry accumulatorTexture = {
		.binding = 0,
		.storageTexture = {
			.access = wgpu::StorageTextureAccess::ReadWrite,
			.format = accumulatorTextureFormat,
			.viewDimension = wgpu::TextureViewDimension::e2D,
		},
	};
	wgpu::BindGroupLayoutEntry infoBuffer = { //this will have the same amount of elements as the screen size
		.binding = 1,
		.buffer = {
			.type = wgpu::BufferBindingType::ReadOnlyStorage,
			.minBindingSize = sizeof(DawnEngine::InfoBufferLayout)
		},
	};
	wgpu::BindGroupLayoutEntry inputTexture = {
		.binding = 2,
		.texture = {
			.sampleType = wgpu::TextureSampleType::Float,
			.viewDimension = wgpu::TextureViewDimension::e2D,
		},
	};
	wgpu::BindGroupLayoutEntry inputSampler = {
		.binding = 3,
		.sampler = {
			.type = wgpu::SamplerBindingType::Filtering, //Use case of non filtering samplers?
		},
	};
	std::array<wgpu::BindGroupLayoutEntry, 4>  bindGroupLayoutEntries = {
		accumulatorTexture,
		infoBuffer,
		inputTexture,
		inputSampler,
	};
	wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
		.label = "texture bind group layout",
		.entryCount = bindGroupLayoutEntries.size(),
		.entries = bindGroupLayoutEntries.data(),
	};
	return _device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);
}
