#include <webgpu/webgpu_cpp.h>

namespace structs {
	namespace device {
		
		//		struct TextureViews {
//			std::vector<wgpu::TextureView> shadowMaps;
//			wgpu::TextureView cameraDepth;
//			wgpu::TextureView textures;
//		};
//
//		struct Samplers {
//			wgpu::Sampler depth;
//			wgpu::Sampler texture;
//		};
//
//		struct BindGroups {
//			wgpu::BindGroup fixed;
//			wgpu::BindGroup lights;
//		};
//
//		struct RenderPipelines {
//			wgpu::RenderPipeline shadow;
//			wgpu::RenderPipeline geometry;
//		};

		struct Objects {
			wgpu::Buffer vbo;
			wgpu::Buffer transforms;
			wgpu::Buffer indices;
			wgpu::Buffer instanceProperties;

			wgpu::Buffer lights;
			wgpu::Buffer cameras;

			wgpu::Buffer materials;
			wgpu::Buffer samplerTexturePaths;
			wgpu::Buffer textureUris;
			wgpu::Buffer samplers;
		};

	}
}