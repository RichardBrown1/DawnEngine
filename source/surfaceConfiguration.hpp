#include <webgpu/webgpu_cpp.h>

namespace surfaceConfiguration {
	wgpu::SurfaceConfiguration self;

	void init(wgpu::Device device, wgpu::Extent2D dimensions) {
		self = wgpu::SurfaceConfiguration{
		.device = device,
		.format = wgpu::TextureFormat::RGBA8Unorm,
		.usage = wgpu::TextureUsage::RenderAttachment,
		.width = dimensions.width,
		.height = dimensions.height,
		.alphaMode = wgpu::CompositeAlphaMode::Auto,
		.presentMode = wgpu::PresentMode::Immediate,
		};
	};
};