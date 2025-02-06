#pragma once
#include <vector>
#include <webgpu/webgpu_cpp.h>
#include "../include/structs.hpp"


//Consider RenderBundles
namespace RenderPipelineHelper {

	struct RenderPipelineHelperDescriptor {
		wgpu::Device& device;
		Buffers& buffers;
		BindGroups& bindGroups;
		wgpu::ShaderModule& vertexShaderModule;
		wgpu::ShaderModule& fragmentShaderModule;
		wgpu::TextureFormat& colorTargetStateFormat;
	};

	wgpu::RenderPipeline createRenderPipeline(RenderPipelineHelperDescriptor& descriptor);
};
