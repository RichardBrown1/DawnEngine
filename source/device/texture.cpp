#pragma once
#include "texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace {
	void createTextureView(
		const wgpu::Device* device,
		const std::string label,
		const wgpu::TextureUsage usage,
		const wgpu::Extent2D dimensions,
		const wgpu::TextureFormat format,
		wgpu::Texture& outTexture,
		wgpu::TextureView& outView
	) {
		assert(format != wgpu::TextureFormat::Undefined);
		const wgpu::TextureDescriptor textureDescriptor = {
					.label = wgpu::StringView(std::string(label) + std::string(" texture")),
					.usage = usage,
					.dimension = wgpu::TextureDimension::e2D,
					.size = {
						.width = dimensions.width,
						.height = dimensions.height,
					},
					.format = format,
		};
		outTexture = device->CreateTexture(&textureDescriptor);
		const wgpu::TextureViewDescriptor textureViewDescriptor = {
			.label = wgpu::StringView(std::string(label) + std::string(" texture view")),
			.format = format,
			.dimension = wgpu::TextureViewDimension::e2D,
			.mipLevelCount = 1,
			.arrayLayerCount = 1,
			.aspect = wgpu::TextureAspect::All,
			.usage = usage,
		};
		outView = outTexture.CreateView(&textureViewDescriptor);
	}

	void getTextureView(
		const wgpu::Device* device,
		const std::string& filePath,
		wgpu::TextureFormat& outFormat,
		wgpu::Texture& outTexture,
		wgpu::TextureView& outTextureView
	)
	{
		constexpr int REQUESTED_CHANNELS = 4;
		int x = 0;
		int y = 0;
		int c = 0;
		unsigned char* data = stbi_load(filePath.c_str(), &x, &y, &c, REQUESTED_CHANNELS);
		const uint32_t width = static_cast<uint32_t>(x);
		const uint32_t height = static_cast<uint32_t>(y);
		const uint32_t channels = static_cast<uint32_t>(REQUESTED_CHANNELS);

		const wgpu::TextureDescriptor textureDescriptor = {
			.label = wgpu::StringView(std::string("texture: " + filePath)),
			.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst,
			.dimension = wgpu::TextureDimension::e2D,
			.size = wgpu::Extent3D {
				.width = width,
				.height = height,
			},
			.format = [channels]() {
				switch (channels) {
				case 1:
					return wgpu::TextureFormat::R32Float;
				case 2:
					return wgpu::TextureFormat::RG32Float;
				default:
					return wgpu::TextureFormat::RGBA8Unorm;
				}
			}(),
			.mipLevelCount = 1,
		};
		outFormat = textureDescriptor.format;
		outTexture = device->CreateTexture(&textureDescriptor);

		const wgpu::TexelCopyTextureInfo texelCopyTextureInfo = {
			.texture = outTexture,
			.mipLevel = 0,
		};
		const wgpu::TexelCopyBufferLayout texelCopyBufferLayout = {
				.bytesPerRow = width * channels * sizeof(char),
				.rowsPerImage = height,
		};

		device->GetQueue().WriteTexture(
			&texelCopyTextureInfo,
			data,
			width * height * channels * sizeof(char),
			&texelCopyBufferLayout,
			&textureDescriptor.size
		);

		stbi_image_free(data);
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

namespace device {
	Texture::Texture(const wgpu::Device* device, const descriptor::Texture& descriptor)
		: format(descriptor.format),
		visibility(descriptor.visibility),
		sampleType(descriptor.sampleType)
	{
		createTextureView(
			device,
			descriptor.label,
			descriptor.usage,
			descriptor.dimensions,
			this->format,
			this->texture,
			this->view
		);
	}

	Texture::Texture(
		const wgpu::Device* device,
		const descriptor::TextureFromFile& descriptor
	) : visibility(descriptor.visibility) {
		//this->sampleType = sampleType;
		getTextureView(
			device,
			descriptor.filePath,
			this->format,
			this->texture,
			this->view
		);
	}

	wgpu::BindGroupLayoutEntry Texture::generateStorageBindGroupLayoutEntry(
		const uint32_t bindingNumber, 
		const wgpu::StorageTextureAccess access
	) {
		return wgpu::BindGroupLayoutEntry{
			.binding = bindingNumber,
			.visibility = this->visibility,
			.storageTexture = {
				.access = access,
				.format = this->format,
				.viewDimension = this->viewDimension,
			},
		};
	}

	wgpu::BindGroupLayoutEntry Texture::generateBindGroupLayoutEntry(const uint32_t bindingNumber) {
		return wgpu::BindGroupLayoutEntry{
			.binding = bindingNumber,
			.visibility = this->visibility,
			.texture = {
				.sampleType = this->sampleType,
				.viewDimension = this->viewDimension,
			}
		};
	}

	wgpu::BindGroupEntry Texture::generateBindGroupEntry(const uint32_t bindingNumber) {
		return wgpu::BindGroupEntry{
			.binding = bindingNumber,
			.textureView = this->view,
		};
	}
}
