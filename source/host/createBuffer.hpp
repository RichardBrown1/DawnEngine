#include <string>
#include <vector>
#include <webgpu/webgpu_cpp.h>
#include "../engine.hpp"
#include <glm/glm.hpp>

namespace host {
	namespace createBuffer {
		template <typename T>
		wgpu::Buffer storage(
			Engine& engine,
			const std::vector<T> vector,
			const std::string &label
		) {
			const wgpu::BufferDescriptor bufferDescriptor = {
				.label = wgpu::StringView(label + " storage buffer"),
				.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
				.size = sizeof(T) * vector.size(),
			};
			wgpu::Buffer buffer = engine.device.CreateBuffer(&bufferDescriptor);
			engine.queue.WriteBuffer(buffer, 0, vector.data(), bufferDescriptor.size);
			return buffer;
		}

	}
}