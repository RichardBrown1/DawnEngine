#pragma once
#include "buffer.hpp"

namespace device {
	wgpu::BindGroupLayoutEntry Buffer::generateBindGroupLayoutEntry(const uint32_t bindingNumber, const wgpu::BufferBindingType type )
	{
		return wgpu::BindGroupLayoutEntry{
			.binding = bindingNumber,
			.visibility = this->visibility,
			.buffer = {
				.type = type,
				.minBindingSize = this->buffer.GetSize(),
			}
		};
	}

	wgpu::BindGroupEntry Buffer::generateBindGroupEntry(const uint32_t bindingNumber)
	{
		return wgpu::BindGroupEntry{
			.binding = bindingNumber,
			.buffer = this->buffer,
		};
	}
}