#include <webgpu/webgpu_cpp.h>
#include "host/host.hpp"

namespace device {
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