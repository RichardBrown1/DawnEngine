#include <webgpu/webgpu_cpp.h>

namespace surfaceConfiguration {
	wgpu::SurfaceConfiguration get(wgpu::Device device, wgpu::Extent2D dimensions) {
		return wgpu::SurfaceConfiguration{
		.device = device,
		.format = wgpu::TextureFormat::BGRA8Unorm,
		.usage = wgpu::TextureUsage::RenderAttachment,
		.width = dimensions.width,
		.height = dimensions.height,
		.alphaMode = wgpu::CompositeAlphaMode::Auto,
		.presentMode = wgpu::PresentMode::Immediate,
		};
	};
};