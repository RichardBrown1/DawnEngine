#include <webgpu/webgpu_cpp.h>

namespace structs {
	namespace device {
				//this is everything from the gltf file
		struct Objects {
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


	}
}