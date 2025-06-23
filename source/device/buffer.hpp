#pragma once 
#include <string>
#include <vector>
#include <dawn/webgpu_cpp.h>

namespace device {
	struct Buffer {
		wgpu::Buffer buffer;
		wgpu::BufferBindingType type;

		wgpu::BindGroupLayoutEntry generateBindGroupLayoutEntry(
			const uint32_t bindingNumber,
			const wgpu::ShaderStage visibility
		);
		wgpu::BindGroupEntry generateBindGroupEntry(const uint32_t bindingNumber);
		
		//empty
		Buffer(
			wgpu::Device& device,
			const uint32_t size,
			const std::string& label,
			const wgpu::BufferUsage bufferUsage,
			const wgpu::BufferBindingType type
		) : type(type) {
			const wgpu::BufferDescriptor bufferDescriptor = {
				.label = wgpu::StringView(label + " buffer"),
				.usage = bufferUsage,
				.size = size,
			};
			this->buffer = device.CreateBuffer(&bufferDescriptor);
		}

		//single element
		template <typename T>
		Buffer(
			wgpu::Device& device,
			const T& structure,
			const std::string& label,
			const wgpu::BufferUsage bufferUsage,
			const wgpu::BufferBindingType type
		) : type(type) {
			const wgpu::BufferDescriptor bufferDescriptor = {
				.label = wgpu::StringView(label + " buffer"),
				.usage = wgpu::BufferUsage::CopyDst | bufferUsage,
				.size = sizeof(T)
			};
			this->buffer = device.CreateBuffer(&bufferDescriptor);
			device.GetQueue().WriteBuffer(this->buffer, 0, &structure, bufferDescriptor.size);
		}

		//vector
		template <typename T>
		Buffer(
			wgpu::Device& device,
			const std::vector<T> vector,
			const std::string& label,
			const wgpu::BufferUsage bufferUsage,
			const wgpu::BufferBindingType type
		) : type(type) {
			const wgpu::BufferDescriptor bufferDescriptor = {
				.label = wgpu::StringView(label + " buffer"),
				.usage = wgpu::BufferUsage::CopyDst | bufferUsage,
				.size = sizeof(T) * vector.size(),
			};
			this->buffer = device.CreateBuffer(&bufferDescriptor);
			device.GetQueue().WriteBuffer(this->buffer, 0, vector.data(), bufferDescriptor.size);
		}

	};
}