#pragma once
#include <string>
#include <dawn/webgpu_cpp.h>
#include "../wgpuContext/wgpuContext.hpp"
#include "../constants.hpp"

namespace device {
	struct RenderResources {
		RenderResources(WGPUContext* wgpuContext);

		const wgpu::TextureFormat worldPositionTextureFormat = wgpu::TextureFormat::RGBA32Float;
		const wgpu::TextureFormat baseColorTextureFormat = wgpu::TextureFormat::RGBA32Float;
		const wgpu::TextureFormat normalTextureFormat = wgpu::TextureFormat::RGBA32Float; //normal texture can tangent-ized	if I need it
		const wgpu::TextureFormat texCoordTextureFormat = wgpu::TextureFormat::R32Uint; //I would prefer unorm but I think its bugged
		const wgpu::TextureFormat baseColorIdTextureFormat = wgpu::TextureFormat::R32Uint;
		const wgpu::TextureFormat normalIdTextureFormat = wgpu::TextureFormat::R32Uint;
		const wgpu::TextureFormat depthTextureFormat = constants::DEPTH_FORMAT;

		wgpu::TextureView worldPositionTextureView;
		wgpu::TextureView baseColorTextureView;
		wgpu::TextureView normalTextureView;
		wgpu::TextureView texCoordTextureView;
		wgpu::TextureView depthTextureView;
		wgpu::TextureView baseColorIdTextureView;
		wgpu::TextureView normalIdTextureView;

		const std::string worldPositionLabel = std::string("world position info");
		const std::string baseColorLabel = std::string("base color");
		const std::string normalLabel = std::string("normals");
		const std::string texCoordLabel = std::string("texcoord");
		const std::string depthTextureLabel = "depth texture";
		const std::string baseColorIdLabel = "base color id";
		const std::string normalIdLabel = "normal id";

		const wgpu::TextureUsage worldPositionTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
		const wgpu::TextureUsage baseColorTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
		const wgpu::TextureUsage normalTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
		const wgpu::TextureUsage texCoordTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
		const wgpu::TextureUsage baseColorIdTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
		const wgpu::TextureUsage normalIdTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageBinding;
		const wgpu::TextureUsage depthTextureUsage = wgpu::TextureUsage::RenderAttachment;
	};
}
