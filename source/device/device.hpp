#pragma once
#include <dawn/webgpu_cpp.h>
#include <string>
#include <vector>
#include "../wgpuContext/wgpuContext.hpp"

namespace device {
	wgpu::ShaderModule createShaderModule(
		const wgpu::Device& device,
		const wgpu::StringView& label,
		const std::string& filename
	);

	wgpu::ShaderModule createWGSLShaderModule(
		const wgpu::Device& device,
		const wgpu::StringView& label,
		const std::string& filename
	);

	template <typename T>
	wgpu::Buffer createBuffer(
		WGPUContext& wgpuContext,
		const std::vector<T> vector,
		const std::string& label,
		const wgpu::BufferUsage bufferUsage
	) {
		const wgpu::BufferDescriptor bufferDescriptor = {
			.label = wgpu::StringView(label + " buffer"),
			.usage = wgpu::BufferUsage::CopyDst | bufferUsage,
			.size = sizeof(T) * vector.size(),
		};
		wgpu::Buffer buffer = wgpuContext.device.CreateBuffer(&bufferDescriptor);
		wgpuContext.queue.WriteBuffer(buffer, 0, vector.data(), bufferDescriptor.size);
		return buffer;
	}

	template <typename T>
	wgpu::Buffer createBuffer(
		WGPUContext& wgpuContext,
		const T& structure,
		const std::string& label,
		const wgpu::BufferUsage bufferUsage
	) {
		const wgpu::BufferDescriptor bufferDescriptor = {
			.label = wgpu::StringView(label + " buffer"),
			.usage = wgpu::BufferUsage::CopyDst | bufferUsage,
			.size = sizeof(T)
		};
		wgpu::Buffer buffer = wgpuContext.device.CreateBuffer(&bufferDescriptor);
		wgpuContext.queue.WriteBuffer(buffer, 0, &structure, bufferDescriptor.size);
		return buffer;
	}

}