#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <dawn/webgpu_cpp.h>
#include "../wgpuContext/wgpuContext.hpp"
#include "../constants.hpp"
#include "../host/host.hpp"

struct RenderResources {
	RenderResources(WGPUContext* wgpuContext);

	const wgpu::TextureFormat worldPositionTextureFormat = wgpu::TextureFormat::RGBA32Float;
	const wgpu::TextureFormat baseColorTextureFormat = wgpu::TextureFormat::RGBA32Float;
	const wgpu::TextureFormat normalTextureFormat = wgpu::TextureFormat::RGBA32Float; //normal texture can tangent-ized	if I need it
	const wgpu::TextureFormat texCoordTextureFormat = wgpu::TextureFormat::R32Uint; //Packed Unorm16x2
	const wgpu::TextureFormat baseColorIdTextureFormat = wgpu::TextureFormat::R32Uint;
	const wgpu::TextureFormat normalIdTextureFormat = wgpu::TextureFormat::R32Uint;
	const wgpu::TextureFormat depthTextureFormat = constants::DEPTH_FORMAT;

	const wgpu::TextureFormat lightingTextureFormat = wgpu::TextureFormat::R32Uint;
	const wgpu::TextureFormat shadowTextureFormat = constants::DEPTH_FORMAT;
	const wgpu::TextureFormat ultimateTextureFormat = wgpu::TextureFormat::RGBA32Float;

	wgpu::TextureView worldPositionTextureView;
	wgpu::TextureView baseColorTextureView;
	wgpu::TextureView normalTextureView;
	wgpu::TextureView texCoordTextureView;
	wgpu::TextureView baseColorIdTextureView;
	wgpu::TextureView normalIdTextureView;
	wgpu::TextureView depthTextureView;
	wgpu::TextureView lightingTextureView;
	wgpu::TextureView shadowTextureView;
	wgpu::TextureView ultimateTextureView;
};

struct SceneResources {
	SceneResources(WGPUContext* wgpuContext, HostSceneResources& host);

	wgpu::Buffer vbo;
	wgpu::Buffer transforms;
	wgpu::Buffer indices;
	wgpu::Buffer materialIndices; //MaterialId for each instance

	std::vector<wgpu::Buffer> lights;
	wgpu::Buffer cameras;

	wgpu::Buffer materials;
	wgpu::Buffer samplerTexturePairs;

	std::unordered_map<uint32_t, wgpu::Sampler> samplers;
	std::vector<wgpu::Texture> textures;
	std::vector<wgpu::TextureView> textureViews;
};

struct DeviceResources {
	RenderResources* render;
	SceneResources* scene;
};
