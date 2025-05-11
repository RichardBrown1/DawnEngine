#include <webgpu/webgpu_cpp.h>

namespace device {
	namespace structs {
		struct Objects {
			wgpu::Buffer vbo;
			wgpu::Buffer transforms;
			wgpu::Buffer indices;
			wgpu::Buffer instanceProperties;
			//is this host or device? wgpu::Buffer drawCalls;

			wgpu::Buffer lights;
			wgpu::Buffer cameras;

			wgpu::Buffer materials;
			wgpu::Buffer samplerTexturePaths;
			wgpu::Buffer textureUris;
			wgpu::Buffer samplers;
		};

	}
}