#pragma once 
#include <string>
#include <dawn/webgpu_cpp.h>

namespace device {
	namespace descriptor {
		struct Texture {
			std::string label;
			wgpu::Extent2D dimensions;
			wgpu::TextureFormat format;

			wgpu::ShaderStage visibility;
			wgpu::TextureUsage usage;
			wgpu::TextureSampleType sampleType;
		};
		
		struct TextureFromFile {
			std::string filePath;

			wgpu::ShaderStage visibility;
			wgpu::TextureUsage usage;
			wgpu::TextureSampleType sampleType;
		};

	}

	struct Texture {
		wgpu::Texture texture;
		wgpu::TextureView view;
		wgpu::TextureFormat format;
		wgpu::ShaderStage visibility;
		wgpu::TextureSampleType sampleType = wgpu::TextureSampleType::UnfilterableFloat;
		wgpu::TextureViewDimension viewDimension = wgpu::TextureViewDimension::e2D;

		Texture(const wgpu::Device* device, const descriptor::Texture& descriptor);
		Texture(const wgpu::Device* device, const descriptor::TextureFromFile& descriptor);

		wgpu::BindGroupLayoutEntry generateStorageBindGroupLayoutEntry(const uint32_t bindingNumber, const wgpu::StorageTextureAccess access);
		wgpu::BindGroupLayoutEntry generateBindGroupLayoutEntry(const uint32_t bindingNumber);
		wgpu::BindGroupEntry generateBindGroupEntry(const uint32_t bindingNumber);
	};
}
