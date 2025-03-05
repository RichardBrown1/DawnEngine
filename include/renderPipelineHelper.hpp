#pragma once
#include <vector>
#include <iostream>
#include <webgpu/webgpu_cpp.h>
#include "../include/structs.hpp"


//Consider RenderBundles
namespace RenderPipelineHelper {

	struct RenderPipelineHelperDescriptor {
		wgpu::Device& device;
		Buffers& buffers;
		TextureViews& textureViews;
		Samplers& samplers;
		BindGroups& bindGroups;
		wgpu::ShaderModule& vertexShaderModule;
		wgpu::ShaderModule& fragmentShaderModule;
		wgpu::TextureFormat& colorTargetStateFormat;
	};

	wgpu::RenderPipeline createOutputRenderPipeline(RenderPipelineHelperDescriptor& descriptor);
	wgpu::RenderPipeline createShadowRenderPipeline(RenderPipelineHelperDescriptor& descriptor);
};
