#pragma once
#include "texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace texture {
	void createTextureView(const descriptor::CreateTextureView* descriptor) {
		assert(descriptor->textureFormat != wgpu::TextureFormat::Undefined);
		const wgpu::TextureDescriptor textureDescriptor = {
					.label = wgpu::StringView(std::string(descriptor->label) + std::string(" texture")),
					.usage = descriptor->textureUsage,
					.dimension = wgpu::TextureDimension::e2D,
					.size = {
						.width = descriptor->textureDimensions.width,
						.height = descriptor->textureDimensions.height,
					},
					.format = descriptor->textureFormat,
		};
		wgpu::Texture texture = descriptor->device->CreateTexture(&textureDescriptor);
		const wgpu::TextureViewDescriptor textureViewDescriptor = {
			.label = wgpu::StringView(std::string(descriptor->label) + std::string(" texture view")),
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
		outTexture = wgpuContext.device.CreateTexture(&textureDescriptor);

		const wgpu::TexelCopyTextureInfo texelCopyTextureInfo = {
			.texture = outTexture,
			.mipLevel = 0,
		};
		const wgpu::TexelCopyBufferLayout texelCopyBufferLayout = {
				.bytesPerRow = width * channels * sizeof(char),
				.rowsPerImage = height,
		};

		wgpuContext.queue.WriteTexture(
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