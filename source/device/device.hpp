#pragma once
#include <webgpu/webgpu_cpp.h>
#include "../host/host.hpp"
#include <string>
#include <vector>
#include "../engine.hpp"

namespace device {
	struct SceneResources {
		wgpu::Buffer vbo;
		wgpu::Buffer transforms;
		wgpu::Buffer indices;
		wgpu::Buffer instanceProperties;

		wgpu::Buffer lights;
		wgpu::Buffer cameras;

		wgpu::Buffer materials;
		wgpu::Buffer samplerTexturePairs;

		std::vector<wgpu::Sampler> samplers;
		std::vector<wgpu::Texture> textures;
		std::vector<wgpu::TextureView> textureViews;
	};

	template <typename T>
	wgpu::Buffer createBuffer(
		Engine& engine,
		const std::vector<T> vector,
		const std::string& label,
		const wgpu::BufferUsage bufferUsage
	) {
		const wgpu::BufferDescriptor bufferDescriptor = {
			.label = wgpu::StringView(label + " buffer"),
			.usage = wgpu::BufferUsage::CopyDst | bufferUsage,
			.size = sizeof(T) * vector.size(),
		};
		wgpu::Buffer buffer = engine.device.CreateBuffer(&bufferDescriptor);
		engine.queue.WriteBuffer(buffer, 0, vector.data(), bufferDescriptor.size);
		return buffer;
	}
}