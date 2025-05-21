#pragma once
#include <string>
#include <ktx.h>
#include <webgpu/webgpu_cpp.h>
#include <absl/log/log.h>
#include "../wgpuContext/wgpuContext.hpp"	

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

	void createTextureView(const descriptor::CreateTextureView* descriptor);
	void getTexture(WGPUContext& wgpuContext, std::string& filePath, wgpu::Texture& outTexture, wgpu::TextureView& outTextureView);
}
